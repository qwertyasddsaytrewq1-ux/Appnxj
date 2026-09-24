#include <curl/curl.h>

#include "../include/bug_log.h"
#include <jni.h>
#include "../include/java.h"
#include <json/json.hpp>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <random>
#include <vector>

#include "include/obfuscate.h"

bool bValid = false;

static bool IsVpnOrTunnelActive(JNIEnv* env)
{
    if (!env)
        return false;

    bool blocked = false;

    jclass contextCls = env->FindClass("android/content/Context");
    jclass cmCls = env->FindClass("android/net/ConnectivityManager");
    jclass ncCls = env->FindClass("android/net/NetworkCapabilities");

    if (contextCls && cmCls && ncCls) {
        jfieldID cmField = env->GetStaticFieldID(
            contextCls, "CONNECTIVITY_SERVICE", "Ljava/lang/String;"
        );
        jmethodID getSystemService = env->GetMethodID(
            contextCls, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;"
        );

        if (cmField && getSystemService) {
            jstring serviceName = (jstring)env->GetStaticObjectField(contextCls, cmField);
            jobject appContext = nullptr;

            jclass atCls = env->FindClass("android/app/ActivityThread");
            if (atCls) {
                jmethodID currentAT = env->GetStaticMethodID(
                    atCls, "currentActivityThread", "()Landroid/app/ActivityThread;"
                );
                if (currentAT) {
                    jobject at = env->CallStaticObjectMethod(atCls, currentAT);
                    if (at) {
                        jmethodID getApp = env->GetMethodID(
                            atCls, "getApplication", "()Landroid/app/Application;"
                        );
                        if (getApp)
                            appContext = env->CallObjectMethod(at, getApp);
                        env->DeleteLocalRef(at);
                    }
                }
                env->DeleteLocalRef(atCls);
            }

            if (appContext && serviceName) {
                jobject cm = env->CallObjectMethod(appContext, getSystemService, serviceName);
                if (cm) {
                    jmethodID getActiveNetwork = env->GetMethodID(
                        cmCls, "getActiveNetwork", "()Landroid/net/Network;"
                    );
                    jmethodID getCapabilities = env->GetMethodID(
                        cmCls, "getNetworkCapabilities",
                        "(Landroid/net/Network;)Landroid/net/NetworkCapabilities;"
                    );
                    jfieldID vpnTransport = env->GetStaticFieldID(
                        ncCls, "TRANSPORT_VPN", "I"
                    );
                    jmethodID hasTransport = env->GetMethodID(
                        ncCls, "hasTransport", "(I)Z"
                    );

                    if (getActiveNetwork && getCapabilities && vpnTransport && hasTransport) {
                        jobject network = env->CallObjectMethod(cm, getActiveNetwork);
                        if (network) {
                            jobject caps = env->CallObjectMethod(cm, getCapabilities, network);
                            if (caps) {
                                jint vpn = env->GetStaticIntField(ncCls, vpnTransport);
                                blocked = env->CallBooleanMethod(caps, hasTransport, vpn) == JNI_TRUE;
                                env->DeleteLocalRef(caps);
                            }
                            env->DeleteLocalRef(network);
                        }
                    }

                    if (!blocked && getCapabilities && vpnTransport && hasTransport) {
                        jmethodID getAllNetworks = env->GetMethodID(
                            cmCls, "getAllNetworks", "()[Landroid/net/Network;"
                        );
                        if (getAllNetworks) {
                            jobjectArray networks = (jobjectArray)env->CallObjectMethod(
                                cm, getAllNetworks
                            );
                            if (networks) {
                                jsize count = env->GetArrayLength(networks);
                                jint vpn = env->GetStaticIntField(ncCls, vpnTransport);
                                for (jsize i = 0; i < count && !blocked; ++i) {
                                    jobject network = env->GetObjectArrayElement(networks, i);
                                    if (!network)
                                        continue;
                                    jobject caps = env->CallObjectMethod(cm, getCapabilities, network);
                                    if (caps) {
                                        blocked = env->CallBooleanMethod(
                                            caps, hasTransport, vpn
                                        ) == JNI_TRUE;
                                        env->DeleteLocalRef(caps);
                                    }
                                    env->DeleteLocalRef(network);
                                }
                                env->DeleteLocalRef(networks);
                            }
                        }
                    }

                    if (!blocked) {
                        jmethodID getNetworkInfo = env->GetMethodID(
                            cmCls, "getNetworkInfo", "(I)Landroid/net/NetworkInfo;"
                        );
                        jfieldID typeVpn = env->GetStaticFieldID(cmCls, "TYPE_VPN", "I");
                        if (getNetworkInfo && typeVpn) {
                            jint vpnType = env->GetStaticIntField(cmCls, typeVpn);
                            jobject ni = env->CallObjectMethod(cm, getNetworkInfo, vpnType);
                            if (ni) {
                                jclass niCls = env->GetObjectClass(ni);
                                jmethodID isConnected = env->GetMethodID(niCls, "isConnected", "()Z");
                                if (isConnected)
                                    blocked = env->CallBooleanMethod(ni, isConnected) == JNI_TRUE;
                                env->DeleteLocalRef(niCls);
                                env->DeleteLocalRef(ni);
                            }
                        }
                    }

                    env->DeleteLocalRef(cm);
                }
            }

            if (serviceName)
                env->DeleteLocalRef(serviceName);
            if (appContext)
                env->DeleteLocalRef(appContext);
        }
    }

    if (!blocked) {
        const char* tunnelNames[] = {
            "tun0", "tun1", "tun2", "tun3", "tun4", "tun5",
            "wg0", "wg1", "wg2", "ppp0", "ppp1",
            "ipsec0", "ipsec1"
        };

        FILE* fp = fopen("/proc/net/dev", "r");
        if (fp) {
            char line[512];
            while (fgets(line, sizeof(line), fp)) {
                for (const char* name : tunnelNames) {
                    if (strstr(line, name)) {
                        blocked = true;
                        break;
                    }
                }
                if (blocked)
                    break;
            }
            fclose(fp);
        }
    }

    if (contextCls) env->DeleteLocalRef(contextCls);
    if (cmCls) env->DeleteLocalRef(cmCls);
    if (ncCls) env->DeleteLocalRef(ncCls);

    return blocked;
}

static time_t g_ServerExpiry = 0;

std::string xor_encrypt(const std::string& data, const std::string& key) {
    std::string result;
    result.reserve(data.size());

    if (key.empty())
        return data;

    for (size_t i = 0; i < data.size(); ++i) {
        result += data[i] ^ key[i % key.length()];
    }

    return result;
}

std::string xor_decrypt(const std::string& data, const std::string& key) {
    return xor_encrypt(data, key);
}

std::string base64_encode(const std::string& data) {
    static const char* chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string result;
    size_t i = 0;

    unsigned char a3[3];
    unsigned char a4[4];

    while (data.length() - i >= 3) {
        a3[0] = static_cast<unsigned char>(data[i]);
        a3[1] = static_cast<unsigned char>(data[i + 1]);
        a3[2] = static_cast<unsigned char>(data[i + 2]);

        a4[0] = (a3[0] & 0xfc) >> 2;
        a4[1] = ((a3[0] & 0x03) << 4) +
                ((a3[1] & 0xf0) >> 4);
        a4[2] = ((a3[1] & 0x0f) << 2) +
                ((a3[2] & 0xc0) >> 6);
        a4[3] = a3[2] & 0x3f;

        for (int j = 0; j < 4; j++)
            result += chars[a4[j]];

        i += 3;
    }

    if (data.length() - i > 0) {
        int remaining =
            static_cast<int>(data.length() - i);

        for (int j = 0; j < 3; j++)
            a3[j] =
                (j < remaining)
                    ? static_cast<unsigned char>(data[i + j])
                    : 0;

        a4[0] = (a3[0] & 0xfc) >> 2;
        a4[1] = ((a3[0] & 0x03) << 4) +
                ((a3[1] & 0xf0) >> 4);
        a4[2] = ((a3[1] & 0x0f) << 2) +
                ((a3[2] & 0xc0) >> 6);
        a4[3] = a3[2] & 0x3f;

        for (int j = 0; j < remaining + 1; j++)
            result += chars[a4[j]];

        while (result.length() % 4)
            result += '=';
    }

    return result;
}

std::string base64_decode(const std::string& input) {
    static const std::string chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string result;

    std::vector<int> T(256, -1);

    for (int i = 0; i < 64; i++)
        T[static_cast<unsigned char>(chars[i])] = i;

    int val = 0;
    int valb = -8;

    for (unsigned char c : input) {
        if (T[c] == -1)
            break;

        val = (val << 6) + T[c];
        valb += 6;

        if (valb >= 0) {
            result.push_back(
                char((val >> valb) & 0xFF)
            );

            valb -= 8;
        }
    }

    return result;
}

static time_t ParseExpiryTime(const std::string& exp)
{
    if (exp.empty() || exp == "N/A")
        return 0;

    try {
        size_t pos = 0;

        long long timestamp =
            std::stoll(exp, &pos);

        if (pos == exp.size() &&
            timestamp > 1000000000LL)
        {
            return static_cast<time_t>(timestamp);
        }
    }
    catch (...) {
    }

    {
        std::tm tm{};
        std::istringstream ss(exp);

        ss >> std::get_time(
            &tm,
            "%Y-%m-%d %H:%M:%S"
        );

        if (!ss.fail()) {
            tm.tm_isdst = -1;

            return timegm(&tm);
        }
    }

    {
        std::tm tm{};
        std::istringstream ss(exp);

        ss >> std::get_time(
            &tm,
            "%Y-%m-%d"
        );

        if (!ss.fail()) {
            tm.tm_isdst = -1;

            return timegm(&tm);
        }
    }

    return 0;
}

INLINE std::string getDt(int offsetSeconds = 0) {

    using namespace std::chrono;

    system_clock::time_point now =
        system_clock::now();

    system_clock::time_point ist =
        now + hours(3) +
        seconds(offsetSeconds);

    std::time_t t =
        system_clock::to_time_t(ist);

    std::tm tm{};

    gmtime_r(&t, &tm);

    std::ostringstream oss;

    oss << std::put_time(
        &tm,
        "%Y-%m-%d %H:%M:%S"
    );

    return oss.str();
}

INLINE std::string gToken(
    const std::string& data,
    const std::string& key)
{
    std::string encrypted =
        xor_encrypt(data, key);

    std::string encoded =
        base64_encode(encrypted);

    return encoded;
}

INLINE std::string decryptData(
    const std::string& encryptedData,
    const std::string& key)
{
    try {

        auto jsonObj =
            nlohmann::json::parse(encryptedData);

        if (!jsonObj.contains("data") ||
            !jsonObj["data"].is_string())
        {
            return "";
        }

        std::string encoded =
            jsonObj["data"].get<std::string>();

        std::string decoded =
            base64_decode(encoded);

        return xor_decrypt(decoded, key);

    }
    catch (...) {

        return "";
    }
}

struct WebSocketFrame {

    std::string data;
    bool success;
};

inline std::string generateWebSocketKey() {

    static const char charset[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string key;

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<> dis(
        0,
        63
    );

    for (int i = 0; i < 16; i++) {

        key += charset[dis(gen)];
    }

    return base64_encode(key);
}

inline std::string createWebSocketFrame(
    const std::string& message)
{
    std::string frame;

    frame += (char)0x81;

    size_t len =
        message.length();

    if (len <= 125) {

        frame +=
            (char)(0x80 | len);

    }
    else if (len <= 65535) {

        frame +=
            (char)(0x80 | 126);

        frame +=
            (char)((len >> 8) & 0xFF);

        frame +=
            (char)(len & 0xFF);

    }
    else {

        frame +=
            (char)(0x80 | 127);

        for (int i = 7; i >= 0; i--) {

            frame +=
                (char)((len >>
                        (i * 8)) & 0xFF);
        }
    }

    unsigned char mask[4];

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<> dis(
        0,
        255
    );

    for (int i = 0; i < 4; i++) {

        mask[i] = dis(gen);

        frame += mask[i];
    }

    for (size_t i = 0;
         i < message.length();
         i++)
    {
        frame +=
            message[i] ^
            mask[i % 4];
    }

    return frame;
}

inline WebSocketFrame parseWebSocketFrame(
    const char* data,
    size_t dataLen)
{
    WebSocketFrame result;

    result.success = false;

    if (dataLen < 2)
        return result;

    size_t payloadLen =
        data[1] & 0x7F;

    size_t headerLen = 2;

    if (payloadLen == 126) {

        if (dataLen < 4)
            return result;

        payloadLen =
            ((unsigned char)data[2] << 8) |
            (unsigned char)data[3];

        headerLen = 4;

    }
    else if (payloadLen == 127) {

        if (dataLen < 10)
            return result;

        payloadLen = 0;

        for (int i = 0; i < 8; i++) {

            payloadLen =
                (payloadLen << 8) |
                (unsigned char)data[2 + i];
        }

        headerLen = 10;
    }

    if (dataLen <
        headerLen + payloadLen)
    {
        return result;
    }

    result.data =
        std::string(
            data + headerLen,
            payloadLen
        );

    result.success = true;

    return result;
}

inline int connectWebSocket(
    const char* host,
    int port,
    int timeoutSec = 10)
{
    int sock =
        socket(
            AF_INET,
            SOCK_STREAM,
            0
        );

    if (sock < 0)
        return -1;

    struct timeval timeout;

    timeout.tv_sec =
        timeoutSec;

    timeout.tv_usec = 0;

    setsockopt(
        sock,
        SOL_SOCKET,
        SO_RCVTIMEO,
        &timeout,
        sizeof(timeout)
    );

    setsockopt(
        sock,
        SOL_SOCKET,
        SO_SNDTIMEO,
        &timeout,
        sizeof(timeout)
    );

    struct sockaddr_in serverAddr;

    memset(
        &serverAddr,
        0,
        sizeof(serverAddr)
    );

    serverAddr.sin_family =
        AF_INET;

    serverAddr.sin_port =
        htons(port);

    if (inet_pton(
            AF_INET,
            host,
            &serverAddr.sin_addr) <= 0)
    {
        close(sock);

        return -1;
    }

    if (connect(
            sock,
            (struct sockaddr*)&serverAddr,
            sizeof(serverAddr)) < 0)
    {
        close(sock);

        return -1;
    }

    std::string wsKey =
        generateWebSocketKey();

    std::stringstream request;

    request
        << "GET / HTTP/1.1\r\n";

    request
        << "Host: "
        << host
        << ":"
        << port
        << "\r\n";

    request
        << "Upgrade: websocket\r\n";

    request
        << "Connection: Upgrade\r\n";

    request
        << "Sec-WebSocket-Key: "
        << wsKey
        << "\r\n";

    request
        << "Sec-WebSocket-Version: 13\r\n";

    request
        << "\r\n";

    std::string reqStr =
        request.str();

    if (send(
            sock,
            reqStr.c_str(),
            reqStr.length(),
            0) < 0)
    {
        close(sock);

        return -1;
    }

    char buffer[1024];

    ssize_t received =
        recv(
            sock,
            buffer,
            sizeof(buffer) - 1,
            0
        );

    if (received <= 0) {

        close(sock);

        return -1;
    }

    buffer[received] = '\0';

    if (strstr(
            buffer,
            "101") == nullptr)
    {
        close(sock);

        return -1;
    }

    return sock;
}

inline std::string sendWebSocketMessage(
    int sock,
    const std::string& message,
    int timeoutSec = 10)
{
    std::string frame =
        createWebSocketFrame(message);

    if (send(
            sock,
            frame.c_str(),
            frame.length(),
            0) < 0)
    {
        return "";
    }

    char buffer[65536];

    ssize_t received =
        recv(
            sock,
            buffer,
            sizeof(buffer),
            0
        );

    if (received <= 0)
        return "";

    WebSocketFrame wsFrame =
        parseWebSocketFrame(
            buffer,
            received
        );

    if (!wsFrame.success)
        return "";

    return wsFrame.data;
}

std::string ERROR_MESSAGE = "";

static bool logged_in = false;
static bool is_logging_in = false;

std::string g_Token;
std::string g_Auth;

std::string g_ExpTime = "N/A";

struct MemoryStruct {

    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(
    void *contents,
    size_t size,
    size_t nmemb,
    void *userp)
{
    const size_t realsize =
        size * nmemb;

    auto *mem =
        static_cast<MemoryStruct*>(
            userp
        );

    char *ptr =
        static_cast<char*>(
            realloc(
                mem->memory,
                mem->size +
                realsize +
                1
            )
        );

    if (!ptr)
        return 0;

    mem->memory = ptr;

    memcpy(
        &(mem->memory[mem->size]),
        contents,
        realsize
    );

    mem->size +=
        realsize;

    mem->memory[mem->size] =
        0;

    return realsize;
}

static std::string GetDarkDeviceUUID(
    JNIEnv *env,
    const std::string &seed)
{
    if (!env ||
        seed.empty())
    {
        return "";
    }

    jclass uuidClass =
        env->FindClass(
            "java/util/UUID"
        );

    if (!uuidClass)
        return "";

    jmethodID nameUUIDFromBytes =
        env->GetStaticMethodID(
            uuidClass,
            "nameUUIDFromBytes",
            "([B)Ljava/util/UUID;"
        );

    jmethodID toString =
        env->GetMethodID(
            uuidClass,
            "toString",
            "()Ljava/lang/String;"
        );

    if (!nameUUIDFromBytes ||
        !toString)
    {
        env->DeleteLocalRef(
            uuidClass
        );

        return "";
    }

    jbyteArray bytes =
        env->NewByteArray(
            static_cast<jsize>(
                seed.size()
            )
        );

    if (!bytes) {

        env->DeleteLocalRef(
            uuidClass
        );

        return "";
    }

    env->SetByteArrayRegion(
        bytes,
        0,
        static_cast<jsize>(
            seed.size()
        ),
        reinterpret_cast<
            const jbyte*
        >(seed.data())
    );

    jobject uuidObj =
        env->CallStaticObjectMethod(
            uuidClass,
            nameUUIDFromBytes,
            bytes
        );

    std::string result;

    if (uuidObj) {

        jstring value =
            static_cast<jstring>(
                env->CallObjectMethod(
                    uuidObj,
                    toString
                )
            );

        if (value) {

            const char *chars =
                env->GetStringUTFChars(
                    value,
                    nullptr
                );

            if (chars) {

                result = chars;

                env->ReleaseStringUTFChars(
                    value,
                    chars
                );
            }

            env->DeleteLocalRef(
                value
            );
        }

        env->DeleteLocalRef(
            uuidObj
        );
    }

    env->DeleteLocalRef(
        bytes
    );

    env->DeleteLocalRef(
        uuidClass
    );

    return result;
}

static std::string GetDarkBuildString(
    JNIEnv *env,
    const char *fieldName)
{
    if (!env ||
        !fieldName)
    {
        return "";
    }

    jclass buildClass =
        env->FindClass(
            "android/os/Build"
        );

    if (!buildClass)
        return "";

    jfieldID field =
        env->GetStaticFieldID(
            buildClass,
            fieldName,
            "Ljava/lang/String;"
        );

    if (!field) {

        env->DeleteLocalRef(
            buildClass
        );

        return "";
    }

    jstring value =
        static_cast<jstring>(
            env->GetStaticObjectField(
                buildClass,
                field
            )
        );

    std::string result;

    if (value) {

        const char *chars =
            env->GetStringUTFChars(
                value,
                nullptr
            );

        if (chars) {

            result = chars;

            env->ReleaseStringUTFChars(
                value,
                chars
            );
        }

        env->DeleteLocalRef(
            value
        );
    }

    env->DeleteLocalRef(
        buildClass
    );

    return result;
}

static std::string GetKeyRemainingTime()
{
    if (g_ServerExpiry <= 0)
        return "N/A";

    const time_t now =
        time(nullptr);

    if (now >= g_ServerExpiry)
        return "EXPIRED";

    long long remaining =
        static_cast<long long>(
            g_ServerExpiry
        ) -
        static_cast<long long>(
            now
        );

    long long days =
        remaining / 86400;

    remaining %= 86400;

    long long hours =
        remaining / 3600;

    remaining %= 3600;

    long long minutes =
        remaining / 60;

    long long seconds =
        remaining % 60;

    char buffer[128];

    snprintf(
        buffer,
        sizeof(buffer),
        "%lld Days %02lld Hours %02lld Minutes %02lld Seconds",
        days,
        hours,
        minutes,
        seconds
    );

    return std::string(buffer);
}

INLINE bool Login(
    std::string androidID,
    std::string key)
{
    // === LICENSE BYPASS ===
    // Short-circuit: force authenticated state, skip panel call,
    // VPN gate, JNI device-ID lookups, and all network I/O.
    (void)androidID;
    (void)key;

    ERROR_MESSAGE.clear();
    is_logging_in = false;

    g_Token   = "1";
    g_Auth    = "1";
    bValid    = true;
    logged_in = true;

    // 2038-01-19 03:14:07 UTC — safe on both 32-bit and 64-bit time_t.
    g_ServerExpiry = 2147483647LL;
    g_ExpTime      = "2038-01-19 03:14:07";

    bug_log_write("KEYLOGIN: BYPASS ACTIVE — license gate disabled");
    bug_log_flush();

    return true;
}
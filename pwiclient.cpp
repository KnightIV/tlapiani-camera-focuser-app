#include "pwiclient.h"

#include <iostream>

PWI4::PWI4(const std::string &url) : base_url(url) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
}

PWI4::~PWI4() {
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

size_t PWI4::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string PWI4::sendGetRequest(const std::string &endpoint) {
    std::string readBuffer;
    std::string url = base_url + endpoint;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::cerr << "GET request failed: " << curl_easy_strerror(res) << std::endl;
        return "";
    }
    return readBuffer;
}

bool PWI4::sendPostRequest(const std::string &endpoint, const nlohmann::json &postData) {
    std::string readBuffer;
    std::string url = base_url + endpoint;
    std::string jsonData = postData.dump();

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    res = curl_easy_perform(curl);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        std::cerr << "POST request failed: " << curl_easy_strerror(res) << std::endl;
        return false;
    }

    return true;
}

// IPWIClient interface
void PWI4::focuserConnect() {
    sendPostRequest("/focuser/connect", {});
}

void PWI4::focuserDisconnect() {
    sendPostRequest("/focuser/disconnect", {});
}

void PWI4::focuserEnable() {
    sendPostRequest("/focuser/enable", {});
}

void PWI4::focuserDisable() {
    sendPostRequest("/focuser/disable", {});
}

void PWI4::focuserGoto(float target) {
    sendPostRequest("/focuser/goto", { {"position", target} });
}

void PWI4::focuserStop() {
    sendPostRequest("/focuser/stop", {});
}

bool PWI4::focuserIsConnected() {
    std::string response = sendGetRequest("/focuser");
    if (response.empty()) return false;
    try {
        auto jsonData = nlohmann::json::parse(response);
        return jsonData["is_connected"];
    }
    catch (...) {
        return false;
    }
}

bool PWI4::focuserIsEnabled() {
    std::string response = sendGetRequest("/focuser");
    if (response.empty()) return false;
    try {
        auto jsonData = nlohmann::json::parse(response);
        return jsonData["is_enabled"];
    }
    catch (...) {
        return false;
    }
}

bool PWI4::focuserIsMoving() {
    std::string response = sendGetRequest("/focuser");
    if (response.empty()) return false;
    try {
        auto jsonData = nlohmann::json::parse(response);
        return jsonData["is_moving"];
    }
    catch (...) {
        return false;
    }
}

float PWI4::focuserPosition() {
    std::string response = sendGetRequest("/focuser");
    if (response.empty()) return -1.0f;
    try {
        auto jsonData = nlohmann::json::parse(response);
        return jsonData["position"];
    }
    catch (...) {
        return -1.0f;
    }
}

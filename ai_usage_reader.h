// ai_usage_reader.h - Reads real Google Antigravity & AI session tokens and rate limits from ~/.gemini
// Modeled directly after AntigravityUsageReader from Dynamic Ledge
#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>
#include <atomic>
#include <fstream>
#include <filesystem>
#include <chrono>

namespace island_ai {

struct AiUsageMetrics {
    bool hasSession = false;
    std::wstring provider = L"Google Antigravity";
    std::wstring model = L"Gemini 2.5 Pro";
    int inputTokens = 0;
    int outputTokens = 0;
    int cacheTokens = 0;
    int totalTokens = 0;
    int maxContextTokens = 1048576; // 1M context
    float percentRemaining = 100.0f;
    bool isActive = false;
};

class AiUsageReader {
public:
    static AiUsageMetrics GetCurrentMetrics() {
        static AiUsageMetrics s_cachedMetrics;
        static auto s_lastCheck = std::chrono::steady_clock::time_point{};
        auto now = std::chrono::steady_clock::now();
        if (s_lastCheck.time_since_epoch().count() != 0 &&
            std::chrono::duration_cast<std::chrono::seconds>(now - s_lastCheck).count() < 3) {
            return s_cachedMetrics;
        }
        s_lastCheck = now;

        wchar_t userProfile[MAX_PATH] = {};
        if (!GetEnvironmentVariableW(L"USERPROFILE", userProfile, MAX_PATH)) {
            return s_cachedMetrics;
        }

        std::filesystem::path brainDir = std::filesystem::path(userProfile) / L".gemini" / L"antigravity" / L"brain";
        if (!std::filesystem::exists(brainDir)) {
            return s_cachedMetrics;
        }

        std::filesystem::path latestTranscript;
        std::filesystem::file_time_type latestTime{};
        try {
            for (const auto& entry : std::filesystem::directory_iterator(brainDir)) {
                if (entry.is_directory()) {
                    auto tPath = entry.path() / L".system_generated" / L"logs" / L"transcript.jsonl";
                    if (std::filesystem::exists(tPath)) {
                        auto ftime = std::filesystem::last_write_time(tPath);
                        if (ftime > latestTime) {
                            latestTime = ftime;
                            latestTranscript = tPath;
                        }
                    }
                }
            }
        } catch (...) {
            return s_cachedMetrics;
        }

        if (latestTranscript.empty()) {
            return s_cachedMetrics;
        }

        // Read last chunk of transcript file
        std::ifstream file(latestTranscript, std::ios::binary);
        if (!file.is_open()) return s_cachedMetrics;
        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        size_t readSize = (size > 16384) ? 16384 : size;
        file.seekg(size - readSize);
        std::string buffer(readSize, '\0');
        file.read(&buffer[0], readSize);

        // Extract model if present
        std::string modelStr = "Gemini 2.5 Pro";
        size_t modelPos = buffer.rfind("\"model\":");
        if (modelPos == std::string::npos) modelPos = buffer.rfind("gemini-");
        if (modelPos != std::string::npos) {
            if (buffer.find("gemini-2.5-flash", modelPos) != std::string::npos) modelStr = "Gemini 2.5 Flash";
            else if (buffer.find("gemini-2.0-flash", modelPos) != std::string::npos) modelStr = "Gemini 2.0 Flash";
            else if (buffer.find("gemini-1.5-pro", modelPos) != std::string::npos) modelStr = "Gemini 1.5 Pro";
            else if (buffer.find("gemini-2.5-pro", modelPos) != std::string::npos) modelStr = "Gemini 2.5 Pro";
        }

        // Parse last line for token stats
        size_t end = buffer.find_last_not_of("\r\n ");
        if (end == std::string::npos) return s_cachedMetrics;
        size_t start = buffer.find_last_of('\n', end);
        std::string line = (start == std::string::npos) ? buffer.substr(0, end + 1) : buffer.substr(start + 1, end - start);

        auto extractInt = [&](const std::string& key) -> int {
            size_t pos = line.find("\"" + key + "\":");
            if (pos == std::string::npos) return 0;
            pos += key.size() + 3;
            while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) pos++;
            return std::atoi(line.c_str() + pos);
        };

        int inTok = extractInt("input_tokens");
        int outTok = extractInt("output_tokens");
        int cacheTok = extractInt("cache_read_tokens");
        int total = inTok + outTok + cacheTok;

        if (total > 0) {
            s_cachedMetrics.hasSession = true;
            s_cachedMetrics.provider = L"Google Antigravity";
            s_cachedMetrics.model = std::wstring(modelStr.begin(), modelStr.end());
            s_cachedMetrics.inputTokens = inTok;
            s_cachedMetrics.outputTokens = outTok;
            s_cachedMetrics.cacheTokens = cacheTok;
            s_cachedMetrics.totalTokens = total;
            s_cachedMetrics.maxContextTokens = 1048576; // 1M context
            float usedRatio = static_cast<float>(total) / static_cast<float>(s_cachedMetrics.maxContextTokens);
            s_cachedMetrics.percentRemaining = (std::max)(0.0f, (1.0f - usedRatio) * 100.0f);
            s_cachedMetrics.isActive = true;
        }

        return s_cachedMetrics;
    }
};

} // namespace island_ai

//
// Created by Francisco Teixeira on 27/02/2025.
//

#include "yahoo_finance_api.h"

#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>

YahooFinanceAPI::YahooFinanceAPI() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("failed to init curl");
    }
}

YahooFinanceAPI::~YahooFinanceAPI() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

size_t YahooFinanceAPI::WriteCallback(void* buf, size_t size, size_t nmemb, std::string* s) {
    size_t len = size*nmemb;
    try {
        s->append((char*)buf, len);
        return len;
    } catch (std::bad_alloc& e) {
        return 0;
    }
}

std::string YahooFinanceAPI::httpGet(const std::string& url) {
    if (!curl) {
        throw std::runtime_error("failed to init curl");
    }

    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    //mimic a browser request
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36");
    headers = curl_slist_append(headers, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        return "";
    }

    return response;
}

std::optional<std::vector<StockPrice>> YahooFinanceAPI::getHistoricalData(const std::string& symbol, int days) {

    //Get current time and time from 'days' ago´
    auto now = std::chrono::system_clock::now();
    auto past = now - std::chrono::hours(24 * days);

    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto past_time_t = std::chrono::system_clock::to_time_t(past);

    //Yahoo finance API uses Unix timestamps so we need to cast
    auto period1 = static_cast<long>(past_time_t);
    auto period2 = static_cast<long>(now_time_t);

    std::stringstream url;
    url << "https://query1.finance.yahoo.com/v8/finance/chart/"
        << symbol
        <<"?period1=" << period1
        <<"&period2=" << period2
        <<"&interval=1d";

    std::string response = httpGet(url.str());

    if (response.empty()) {
        return std::nullopt;
    }

    try {
        nlohmann::json json = nlohmann::json::parse(response);
        return parseHistoricalData(json);
    } catch (std::exception& e) {
        std::cerr << "exception: " << e.what() << std::endl;
        return std::nullopt;
    }
}

std::vector<StockPrice> YahooFinanceAPI::parseHistoricalData(const nlohmann::json& json) {
    std::vector<StockPrice> parserdHistoricalData;

    if (!json.contains("chart") || !json["chart"].contains("result") || json["chart"]["result"].empty()) {
        return parserdHistoricalData;
    }

    const auto& chart_data = json["chart"]["result"][0];

    if (!chart_data.contains("timestamp") || !chart_data.contains("indicators")) {
        return parserdHistoricalData;
    }

    const auto& timestamps = chart_data["timestamps"];
    const auto& quote = chart_data["indicators"]["quote"][0];
    const auto& adjclose = chart_data["indicators"].contains("adjclose") ?
        chart_data["indicators"]["adjclose"][0]["adjclose"]
        : nlohmann::json::array();

    if (!quote.contains("open") || !quote.contains("high") || !quote.contains("low")
        || !quote.contains("close") || !quote.contains("volume")) {
        return parserdHistoricalData;
    }

    const auto& open = quote["open"];
    const auto& high = quote["high"];
    const auto& low = quote["low"];
    const auto& close = quote["close"];
    const auto& volume = quote["volume"];

    for (size_t i = 0; i < timestamps.size(); i++) {
        if (open[i].is_null() || high[i].is_null() || low[i].is_null() || close[i].is_null() || volume[i].is_null()) {
            continue;
        }

        time_t timestamp = timestamps[i];
        std::tm* tm = std::localtime(&timestamp);
        char buffer[11]; //YYYY-MM-DD\0
        std::strftime(buffer, sizeof(buffer), "%Y:%m:%d", tm);

        StockPrice stockPrice;
        stockPrice.timestamp = std::string(buffer);
        stockPrice.open = open[i];
        stockPrice.high = high[i];
        stockPrice.low = low[i];
        stockPrice.close = close[i];
        stockPrice.volume = volume[i];
        stockPrice.adjclose = (i < adjclose.size() && !adjclose[i].is_null()) ?
            adjclose[i].get<double>() : stockPrice.close;

        parserdHistoricalData.push_back(stockPrice);
    }

    return parserdHistoricalData;
}

std::optional<StockPrice> YahooFinanceAPI::getLatestStockPrice(const std::string& stockId) {
    auto data = getHistoricalData(stockId, 2);
    if (!data || data->empty()) {
        return std::nullopt;
    }

    return  data->back();

}


std::map<std::string, std::vector<StockPrice>> YahooFinanceAPI::getHistoricalDataBatch(const std::vector<std::string>& symbols,
    int days) {

    std::map<std::string, std::vector<StockPrice>> res;

    for (const auto& symbol : symbols) {
        auto data = getHistoricalData(symbol, days);
        if (data) {
            res[symbol] = *data;
        }
    }

    return res;
}


std::vector<std::string> YahooFinanceAPI::getTrendingStocks(int limit) {
    std::string url = "https://query1.finance.yahoo.com/v1/finance/trending/US?count=" + std::to_string(limit);
    std::string response = httpGet(url);

    if (response.empty()) {
        return {};
    }

    try {
        nlohmann::json json = nlohmann::json::parse(response);
        return parseTrendingStocks(json);
    } catch (std::exception& e) {
        std::cerr << "exception: " << e.what() << std::endl;
        return {};
    }
}

std::vector<std::string> YahooFinanceAPI::parseTrendingStocks(const nlohmann::json &json) {
    std::vector<std::string> res;

    if (!json.contains("finance") || !json["finance"].contains("result")
        || !json["finance"]["result"].empty()
        || !json["finance"]["result"][0].contains("quotes")) {
        return res;
    }

    const auto& quotes = json["finance"]["result"][0]["quotes"];

    for (const auto& quote : quotes) {
        if (quote.contains("symbol")) {
            res.push_back(quote["symbol"]);
        }
    }

    return res;
}

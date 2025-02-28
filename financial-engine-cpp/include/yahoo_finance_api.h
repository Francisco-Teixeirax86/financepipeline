//
// Created by Francisco Teixeira on 27/02/2025.
//

#ifndef STOCK_DATA_API_H
#define STOCK_DATA_API_H
#include <string>
#include <vector>
#include <optional>
#include <curl\curl.h>
#include <nlohmann/json.hpp>

struct StockPrice {
    std::string timestamp;
    double open;
    double high;
    double low;
    double close;
    long volume;
    double adjclose;
};

//Using YahooFinance API
class YahooFinanceAPI {
public:
   YahooFinanceAPI();
    ~YahooFinanceAPI();

    //Fetch historical data for a given stockID/symbol
    std::optional<std::vector<StockPrice>> getHistoricalData(
        const std::string& stockId,
        int days = 30
    );

    //Fetch latest price and volume data
    std::optional<StockPrice> getLatestStockPrice(const std::string& stockId);

    std::map<std::string, std::vector<StockPrice>> getHistoricalDataBatch(
        const std::vector<std::string>& symbols,
        int days = 30);


    //Get trending symbols/stockIds based on yahoo finance trending tickers
    std::vector<std::string> getTrendingStocks(int limit = 20);

private:
    CURL* curl;

    static size_t WriteCallback(void* buf, size_t size, size_t nmemb, std::string* userp);
    std::string httpGet(const std::string& url);
    std::vector<StockPrice> parseHistoricalData(const nlohmann::json& json);
    std::vector<std::string> parseTrendingStocks(const nlohmann::json& json);


};

#endif //STOCK_DATA_API_H

//
// Created by Francisco Teixeira on 27/02/2025.
//

#ifndef STOCK_DATA_H
#define STOCK_DATA_H
#include <chrono>

struct StockPrice {
    std::chrono::system_clock::time_point time;
    double open;
    double high;
    double low;
    double close;
    long volume;
};

class StockData {
public:
    StockData(const std::string& symbol);

    bool fetchHistoricalData(int days = 30);
    bool fetchLatestData();

    std::string getSymbol() const;
    std::vector<StockPrice> getStockPrices() const;

    double getChangedPercentage() const;
    double getAverageVolume() const;
private:
    std::string symbol;
    std::vector<StockPrice> stockPrices;


};

#endif //STOCK_DATA_H

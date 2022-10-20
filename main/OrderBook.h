#pragma once
#include "OrderBookEntry.h"
#include "CSVReader.h"
#include <string>
#include <vector>

class OrderBook
{
public:
    OrderBook(std::string filename); // read file
    std::vector<std::string> getKnownProducts(); // return products
    std::vector<OrderBookEntry> getOrders(OrderBookType type, std::string product, std::string timestamp);
    std::vector<OrderBookEntry> matchAsksToBids(std::string product, std::string timestamp); //matching engine to perform matching ask to bid
    void insertOrder(OrderBookEntry& order);
    void withdraw(std::string timestamp);


    std::string getEarliestTime(); // return the earliest time
    std::string getNextTime(std::string timestamp); // return next time
    std::string getPreviousTime(std::string timestamp); // return previous time
    static double getMeanPrice(std::vector <OrderBookEntry>& orders); // get average price
    static double getMeanAmount(std::vector <OrderBookEntry>& orders); // get average amount

private:
    std::vector<OrderBookEntry> orders;
};




#pragma once

#include <vector>
#include "OrderBookEntry.h"
#include "OrderBook.h"
#include "Wallet.h"


class MerkelMain
{
public:
    MerkelMain();
    void init(); // by calling this init function to execute the program

private:
    void printMarketStats();
    void enterAsk();
    void enterBid();
    void printWallet();
    void gotoNextTimeframe();
    void askPercentageChange();
    void bidPercentageChange();

    std::string currentTime;
    std::string previousTime;
    std::string valueBid = "";
    std::string valueAsk = "";

    OrderBook orderBook{ "20200317.csv" }; // csv file
    Wallet wallet;
};


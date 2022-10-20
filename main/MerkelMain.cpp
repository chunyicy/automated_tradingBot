#include "MerkelMain.h"
#include <iostream>
#include <vector>
#include "OrderBookEntry.h"
#include "CSVReader.h"
#include <fstream>
#include <sstream>

MerkelMain::MerkelMain()
{
}

void MerkelMain::init()
{
    currentTime = orderBook.getEarliestTime();
    wallet.insertCurrency("BTC", 200);

    // make the automation to loop 10 times
    for (int counter = 0; counter < 10; counter++)
    {
        printWallet();
        printMarketStats();
        gotoNextTimeframe();
    }
}


void MerkelMain::askPercentageChange() //ASK
{
    for (std::string const& p : orderBook.getKnownProducts())
    {
        previousTime = orderBook.getPreviousTime(currentTime);

        std::cout << "Product: " << p << std::endl;
        std::vector<OrderBookEntry> previousAsk = orderBook.getOrders(OrderBookType::ask,
            p, previousTime);
        std::vector<OrderBookEntry> currentAsk = orderBook.getOrders(OrderBookType::ask,
            p, currentTime);
        double previousAskMeanPrice = orderBook.getMeanPrice(previousAsk);
        double currentAskMeanPrice = orderBook.getMeanPrice(currentAsk);
        //differenceBetweenPreviousCurrentAsk
        double difference = currentAskMeanPrice - previousAskMeanPrice;
        //find the percentage
        double percentageChange = (difference / previousAskMeanPrice) * 100;

        std::cout << "Previous average ask: " << OrderBook::getMeanPrice(previousAsk) << "  Current average ask: " << OrderBook::getMeanPrice(currentAsk) << std::endl;
        std::cout << "Difference: " << difference << std::endl;
        // (+ve sign indicate increment), ( -ve sign indicate decrement)
        std::cout << "Percentage change: " << percentageChange << std::endl;
    }
}


void MerkelMain::bidPercentageChange() //BID
{
    for (std::string const& p : orderBook.getKnownProducts())
    {
        previousTime = orderBook.getPreviousTime(currentTime);

        std::cout << "Product: " << p << std::endl;
        std::vector<OrderBookEntry> previousBid = orderBook.getOrders(OrderBookType::bid, p, previousTime);
        std::vector<OrderBookEntry> currentBid = orderBook.getOrders(OrderBookType::bid, p, currentTime);
        //previous average price
        double previousBidMeanPrice = orderBook.getMeanPrice(previousBid);
        //current average price
        double currentBidMeanPrice = orderBook.getMeanPrice(currentBid);
        //difference Between Previous Current Bid
        double difference = currentBidMeanPrice - previousBidMeanPrice;
        double percentageChange = (difference / previousBidMeanPrice) * 100;
        std::cout << "Previous average bid: " << OrderBook::getMeanPrice(previousBid) << "  Current average Bid: " << OrderBook::getMeanPrice(currentBid) << std::endl;
        // (+ve sign indicate increment), ( -ve sign indicate decrement)
        std::cout << "Difference: " << difference << std::endl;
        std::cout << "Percentage change: " << percentageChange << std::endl;
    }
}


void MerkelMain::printMarketStats()
{
    // calling this function to print out bid market status
    bidPercentageChange();
    // calling this function to print out ask market status
    askPercentageChange();


    for (std::string const& p : orderBook.getKnownProducts())
    {
        previousTime = orderBook.getPreviousTime(currentTime);
        //BID
        std::vector<OrderBookEntry> previousBid = orderBook.getOrders(OrderBookType::bid, p, previousTime);
        std::vector<OrderBookEntry> currentBid = orderBook.getOrders(OrderBookType::bid, p, currentTime);
        double previousBidMeanPrice = orderBook.getMeanPrice(previousBid);
        double currentBidMeanPrice = orderBook.getMeanPrice(currentBid);
        double currentBidAmount = orderBook.getMeanAmount(currentBid);
        double bidDifference = currentBidMeanPrice - previousBidMeanPrice;
        double bidPercentageChange = (bidDifference / previousBidMeanPrice) * 100;

        //ASK
        std::vector<OrderBookEntry> previousAsk = orderBook.getOrders(OrderBookType::ask, p, previousTime);
        std::vector<OrderBookEntry> currentAsk = orderBook.getOrders(OrderBookType::ask, p, currentTime);
        double previousAskMeanPrice = orderBook.getMeanPrice(previousAsk);
        double currentAskMeanPrice = orderBook.getMeanPrice(currentAsk);
        double askDifference = currentAskMeanPrice - previousAskMeanPrice;
        double askPercentageChange = (askDifference / previousAskMeanPrice) * 100;

        // getting the average amount of current Bid and current Ask
        double currentBMeanAmount = orderBook.getMeanAmount(currentBid);
        double currentAMeanAmount = orderBook.getMeanAmount(currentAsk);



        // when the ask percentge is less than 0.05% or when it decrease then make a bid
        if (askPercentageChange < 0.05)
        {
            std::stringstream stream;
            stream << currentAskMeanPrice;
            std::string price = stream.str();
            valueBid = p + "," + price + "," + std::to_string(currentAMeanAmount);
            //calling the enterBid() function to pass in the bid 
            enterBid();
            //output this Bid data to bidAskLogging text file
            std::ofstream print_Bid;
            print_Bid.open("bidAskLogging.txt", std::ofstream::out | std::ofstream::app);
            print_Bid << "Bid: " << valueBid << "\n";
            print_Bid.close();
        }

        // withdraw bid when the ask percentage is more than or equal to 0.05
        if (askPercentageChange > 0.05) {
            orderBook.withdraw(currentTime);
        }

        // when the bid percentage is more than 0.2% then make a Ask
        if (bidPercentageChange > 0.2)
        {
            std::stringstream stream;
            stream << currentBidMeanPrice;
            std::string price = stream.str();
            valueAsk = p + "," + price + "," + std::to_string(currentBMeanAmount);
            //calling the ask function to pass in the Ask
            enterAsk();
            //output Ask to the bidAskLogging text file using ofstream class
            std::ofstream print_Ask;
            print_Ask.open("bidAskLogging.txt", std::ofstream::out | std::ofstream::app);
            print_Ask << "Ask: " << valueAsk << "\n";
            print_Ask.close();
        }

        // withdraw Ask when bid percentage less than or equal to 0.2
        if (bidPercentageChange < 0.2)
        {
            orderBook.withdraw(currentTime);
        }

    }
}

void MerkelMain::enterAsk() // make a ask
{
    std::cout << "Make an ask - enter the amount: product,price, amount, eg  ETH/BTC,200,0.5" << std::endl;
    std::string input = valueAsk;
    // tokenise the input that sent in
    std::vector<std::string> tokens = CSVReader::tokenise(input, ',');
    if (tokens.size() != 3)
    {
        std::cout << "MerkelMain::enterAsk Bad input! " << input << std::endl;
    }
    else {
        try {
            OrderBookEntry obe = CSVReader::stringsToOBE(
                tokens[1],
                tokens[2],
                currentTime,
                tokens[0],
                OrderBookType::ask
            );
            obe.username = "simuser";
            //if wallet have enough
            if (wallet.canFulfillOrder(obe))
            {
                std::cout << "Wallet looks good. " << std::endl;
                orderBook.insertOrder(obe);
            }
            else {
                std::cout << "Wallet has insufficient funds . " << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            std::cout << " MerkelMain::enterAsk Bad input " << std::endl;
        }
    }
}

void MerkelMain::enterBid() // make a bid
{
    std::cout << "Make an bid - enter the amount: product,price, amount, eg  ETH/BTC,200,0.5" << std::endl;
    std::string input = valueBid;
    std::vector<std::string> tokens = CSVReader::tokenise(input, ',');
    if (tokens.size() != 3)
    {
        std::cout << "MerkelMain::enterBid Bad input! " << input << std::endl;
    }
    else {
        try {
            OrderBookEntry obe = CSVReader::stringsToOBE(
                tokens[1],
                tokens[2],
                currentTime,
                tokens[0],
                OrderBookType::bid
            );
            obe.username = "simuser";

            if (wallet.canFulfillOrder(obe))
            {
                std::cout << "Wallet looks good. " << std::endl;
                orderBook.insertOrder(obe);
            }
            else {
                std::cout << "Wallet has insufficient funds . " << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            std::cout << " MerkelMain::enterBid Bad input " << std::endl;
        }
    }
}

void MerkelMain::printWallet()
{
    std::cout << wallet.toString() << std::endl;
    // maintain a record of the updates on the wallet by output these data into assetLogging text file
    //asset records
    std::ofstream print_Wallet;
    print_Wallet.open("assetLogging.txt", std::ofstream::out | std::ofstream::app);
    print_Wallet << "simUser assets " << "\n" << wallet.toString() << "\n";
    print_Wallet.close();
}

void MerkelMain::gotoNextTimeframe()
{
    std::cout << "Going to next time frame. " << std::endl;
    for (std::string p : orderBook.getKnownProducts())
    {
        std::cout << "matching " << p << std::endl;
        std::vector<OrderBookEntry> sales = orderBook.matchAsksToBids(p, currentTime);
        std::cout << "Sales: " << sales.size() << std::endl;
        for (OrderBookEntry& sale : sales)
        {
            std::cout << "Sale price: " << sale.price << " amount " << sale.amount << std::endl;
            if (sale.username == "simuser")
            {
                // to get wallet updated
                wallet.processSale(sale);

            }
        }

    }
    currentTime = orderBook.getNextTime(currentTime);
}


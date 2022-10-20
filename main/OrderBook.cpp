#include "OrderBook.h"
#include "CSVReader.h"
#include <map>
#include <algorithm>
#include <iostream>

OrderBook::OrderBook(std::string filename) // read the data file
{
    orders = CSVReader::readCSV(filename);
}


std::vector<std::string> OrderBook::getKnownProducts() // return all the products
{
    std::vector<std::string> products;
    std::map<std::string, bool> prodMap;

    for (OrderBookEntry& e : orders)
    {
        prodMap[e.product] = true;
    }
    for (auto const& e : prodMap)
    {
        products.push_back(e.first);
    }
    return products;
}


//return type, product and timestamp 
std::vector<OrderBookEntry> OrderBook::getOrders(OrderBookType type, std::string product, std::string timestamp)
{
    std::vector<OrderBookEntry> orders_sub;
    for (OrderBookEntry& e : orders)
    {
        if (e.orderType == type &&
            e.product == product &&
            e.timestamp == timestamp)
        {
            orders_sub.push_back(e);
        }

    }
    return orders_sub;
}


double OrderBook::getMeanPrice(std::vector <OrderBookEntry>& orders) //get average prices
{
    double sumOfPrices = 0;
    for (OrderBookEntry& order : orders)
    {
        sumOfPrices += order.price;
    }
    return sumOfPrices / orders.size();
}


double OrderBook::getMeanAmount(std::vector <OrderBookEntry>& orders) //return average amount
{
    double sumOfAmount = 0;
    for (OrderBookEntry& order : orders)
    {
        sumOfAmount += order.amount;
    }
    return sumOfAmount / orders.size();
}

std::string OrderBook::getEarliestTime()
{
    return orders[0].timestamp;
}


std::string OrderBook::getNextTime(std::string timestamp)
{
    std::string next_timestamp = "";
    for (OrderBookEntry& e : orders)
    {
        if (e.timestamp > timestamp)
        {
            next_timestamp = e.timestamp;
            break;
        }
    }
    if (next_timestamp == "")
    {
        next_timestamp = orders[0].timestamp;
    }
    return next_timestamp;
}


std::string OrderBook::getPreviousTime(std::string timestamp)
{
    std::string previous_timestamp = "";
    for (OrderBookEntry& e : orders)
    {
        if (e.timestamp < timestamp)
        {
            previous_timestamp = e.timestamp;
        }
    }
    if (previous_timestamp == "")
    {
        previous_timestamp = orders[0].timestamp;
    }
    return previous_timestamp;
}


void OrderBook::insertOrder(OrderBookEntry& order)
{
    orders.push_back(order);
    std::sort(orders.begin(), orders.end(), OrderBookEntry::compareByTimestamp); // arrange the order by timestamp
}


std::vector<OrderBookEntry> OrderBook::matchAsksToBids(std::string product, std::string timestamp)
{
    // asks = orderbook.asks
    std::vector<OrderBookEntry> asks = getOrders(OrderBookType::ask, product, timestamp);
    // bids = orderbook.bids
    std::vector<OrderBookEntry> bids = getOrders(OrderBookType::bid, product, timestamp);
    // sales = []
    std::vector<OrderBookEntry> sales;
    if (asks.size() == 0 || bids.size() == 0)
    {
        std::cout << " OrderBook::matchAsksToBids no bids or asks" << std::endl;
        return sales;
    }
    // sort asks lowest first
    std::sort(asks.begin(), asks.end(), OrderBookEntry::compareByPriceAsc);
    // sort bids highest first
    std::sort(bids.begin(), bids.end(), OrderBookEntry::compareByPriceDesc);

    // for ask in asks:
    std::cout << "max ask " << asks[asks.size() - 1].price << std::endl;
    std::cout << "min ask " << asks[0].price << std::endl;
    std::cout << "max bid " << bids[0].price << std::endl;
    std::cout << "min bid " << bids[bids.size() - 1].price << std::endl;

    for (OrderBookEntry& ask : asks)
    {
        //     for bid in bids:
        for (OrderBookEntry& bid : bids)
        {
            //         if bid.price >= ask.price # we have a match
            if (bid.price >= ask.price)
            {
                //             sale = new order()
                //             sale.price = ask.price
                OrderBookEntry sale{ ask.price, 0, timestamp,
                    product,
                    OrderBookType::asksale };

                if (bid.username == "simuser")
                {
                    sale.username = "simuser";
                    sale.orderType = OrderBookType::bidsale;
                }
                if (ask.username == "simuser")
                {
                    sale.username = "simuser";
                    sale.orderType = OrderBookType::asksale;
                }
                //             # now work out how much was sold and 
                //             # create new bids and asks covering 
                //             # anything that was not sold
                //             if bid.amount == ask.amount: # bid completely clears ask
                if (bid.amount == ask.amount)
                {
                    //                 sale.amount = ask.amount
                    sale.amount = ask.amount;
                    //                 sales.append(sale)
                    sales.push_back(sale);
                    //                 bid.amount = 0 # make sure the bid is not processed again
                    bid.amount = 0;
                    //                 # can do no more with this ask
                    //                 # go onto the next ask
                    //                 break
                    break;
                }
                //           if bid.amount > ask.amount:  # ask is completely gone slice the bid
                if (bid.amount > ask.amount)
                {
                    //                 sale.amount = ask.amount
                    sale.amount = ask.amount;
                    //                 sales.append(sale)
                    sales.push_back(sale);
                    //                 # we adjust the bid in place
                    //                 # so it can be used to process the next ask
                    //                 bid.amount = bid.amount - ask.amount
                    bid.amount = bid.amount - ask.amount;
                    //                 # ask is completely gone, so go to next ask                
                    //                 break
                    break;
                }
                //             if bid.amount < ask.amount # bid is completely gone, slice the ask
                if (bid.amount < ask.amount &&
                    bid.amount > 0)
                {
                    //                 sale.amount = bid.amount
                    sale.amount = bid.amount;
                    //                 sales.append(sale)
                    sales.push_back(sale);
                    //                 # update the ask
                    //                 # and allow further bids to process the remaining amount
                    //                 ask.amount = ask.amount - bid.amount
                    ask.amount = ask.amount - bid.amount;
                    //                 bid.amount = 0 # make sure the bid is not processed again
                    bid.amount = 0;
                    //                 # some ask remains so go to the next bid
                    //                 continue
                    continue;
                }
            }
        }
    }
    return sales;
}

// this is the withdraw order function
void OrderBook::withdraw(std::string timestamp)
{
    signed char i;
    for (i = orders.size(); i < orders.size(); ++i)
    {
        if (orders[i].timestamp == timestamp)
        {
            orders.erase(orders.begin() + i); // using erase funtion to remove the orders
        }
    }
}


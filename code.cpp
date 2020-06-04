#include <bits/stdc++.h>

using namespace std;

string ltrim(const string &);
string rtrim(const string &);
vector<string> split(const string &);

/*
 * Function for checking if two doubles are equal.
 */
bool areEqual(double a, double b)
{
    double EPSILON = 0.0000000001;
    return fabs(a - b) < EPSILON;
}

/*
 * Class for storing a single ETF's component data.
 */
class ETFComponent {
    public:
        string symbol, currency;
        double weight;
        ETFComponent (vector<string>);
};
ETFComponent::ETFComponent (vector<string> component) {
    symbol = component[0];
    weight = stod(component[1]);
    currency = component[2];
};
  
/*
 * Class for storing a single message (containing prices from exchanges) data.
 */
class Message {
    public:
        string type, symbol, exchange;
        double px, qty;
        Message (vector<string>);
};
Message::Message (vector<string> message) {
    type = message[0];
    symbol = message[1];
    exchange = message[2];
    px = stod(message[3]);
    qty = stod(message[4]);
};

/*
 * Class for storing a single ETF's data.
 */
class ETF {
    vector<ETFComponent> components;
    int numberOfListingInETF;
    public:
        string name, currency;
        double bidPx, askPx, bidQty, askQty, lastValidAskPx, lastValidBidPx;
        ETF (string,string,vector<ETFComponent>);
        /*
        * Function finds the exchange rate for a component and (if needed) translates the component's price to the ETF's currency price.
        */    
        double translateToETFCurrency(double px, ETFComponent component, map<string,Message> currencyRate) {
            if (currency == component.currency) {return px;}

            const string currencyPair = currency + component.currency;
            const string inverseCurrencyPair = component.currency + currency;
            if (currencyRate.count(inverseCurrencyPair) > 0) {return px * currencyRate.find(inverseCurrencyPair)->second.px;}
            else if (currencyRate.count(currencyPair) > 0) {return px / currencyRate.find(currencyPair)->second.px;}
            else {return 0;}
        }
        /*
        * Function calculates fair bid price and qty if prices are available, otherwise sets bid price and qty to zero.
        */
        void calcQuoteBid(map<string,Message> bid, map<string,Message> ask, map<string,Message> currencyRate) {
            bidPx = 0;
            for (int i = 0; i < numberOfListingInETF; i++) {
                if (bid.count(components[i].symbol) > 0) { // there is info about such component px in messages
                    bidPx += translateToETFCurrency(bid.find(components[i].symbol)->second.px, components[i], currencyRate) * components[i].weight;
                    // double currentQty = bid.find(components[i].symbol)->second.qty;
                    // if (currentQty < bidQty) {bidQty = currentQty;}
                } else { // there is NO info about such component px in messages
                    bidPx = 0;
                    bidQty = 0;
                    return;
                }
            }
            bidQty = 10; // by definition // bidQty *= numberOfListingInETF;
        }
        /*
        * Function calculates fair ask price and qty if prices are available, otherwise sets ask price and qty to zero.
        */
        void calcQuoteAsk(map<string,Message> bid, map<string,Message> ask, map<string,Message> currencyRate) {
            askPx = 0;
            for (int i = 0; i < numberOfListingInETF; i++) {
                if (ask.count(components[0].symbol) > 0) { // there is info about such component px in messages
                    askPx += translateToETFCurrency(ask.find(components[i].symbol)->second.px, components[i], currencyRate) * components[i].weight;
                // double currentQty = ask.find(components[i].symbol)->second.qty;
                // if (currentQty < askQty) {askQty = currentQty;}
                } else { // there is NO info about such component px in messages
                    askPx = 0;
                    askQty = 0;
                    return;
                }
            askQty = 10; // by definition // askQty *= numberOfListingInETF;
            }
        }
        /*
        * Function checks if two quotes (for bid and ask) can be given (i.e. when a theoretical askPx and bidPx can be calculated).
        */
        bool canSendOrders() {
            if (areEqual(askQty,0) || (areEqual(bidQty,0))) { // the quote cannot be given
                bidQty = 0;
                askQty = 0;
                return 0;
            }
            return 1;
        }
        /*
        * Function checks if bid and ask prices are available (can be found) for all ETF's components.
        */
        bool allComponentsHaveAskBid(map<string,Message> bid, map<string,Message> ask) {
            bool allTrue = 1;
            for (int i = 0; i < numberOfListingInETF; i++) {
                if ((ask.count(components[i].symbol) <= 0) || (bid.count(components[i].symbol) <= 0)) {
                    allTrue = 0;
                    return allTrue;
                }
            }
            return allTrue;
        }
};
ETF::ETF (string name_, string currency_, vector<ETFComponent> components_) {
    name = name_;
    currency = currency_;
    components = components_;
    numberOfListingInETF = components_.size();
    bidPx = 0; // fair bid price
    askPx = 0; // fair ask price
    bidQty = 0;
    askQty = 0;
    lastValidAskPx = 0; // last sent ask px for this ETF
    lastValidBidPx = 0; // last sent bid px for this ETF
};

/*
 * Function updates three maps (bid, ask, currencyRate) with the lates info from the market which came with a message. 
 */
void collectMessageInfo(vector<string> message, map<string,Message>& bid, map<string,Message>& ask, map<string,Message>& currencyRate) {
    Message msg (message);
    if (msg.type == "X") {
        currencyRate.emplace(msg.symbol, msg);
    }
    else if (msg.type == "B") {
        bid.emplace(msg.symbol, msg);
    } else if (msg.type == "A") {
        ask.emplace(msg.symbol, msg);
    } else {
        cout << "Message type not supported: " << msg.type << endl;
    }
};

/*
 * Function tries to print out a quote for each ETF taking into account all the information available from the market by the time this function is launched. If insufficient info available (e.g. ETF's theoretical price cannot be calculated) the function does not print anything.
 */
void playback(vector<ETF>& etfs, map<string, Message> bid, map<string, Message> ask, map<string, Message> currencyRate) {
    int NumberOfETF = etfs.size();
    const double priceTick = 0.01;
    for (int i = 0; i < NumberOfETF; i++) {
        // ETF etf = etfs[i];
                
        // update the quotes
        etfs[i].calcQuoteBid(bid, ask, currencyRate);
        etfs[i].calcQuoteAsk(bid, ask, currencyRate);
        
        // check if the ETF price was calculated and orders can be sent
        bool pxNotZero = etfs[i].canSendOrders();
        
        double outputQty = 0;
        // if prices can be calculated and all components' prices are available and the quote is different from the previous one - send orders with calculated quote
        if ((pxNotZero) && (etfs[i].allComponentsHaveAskBid(bid, ask)) && ((!areEqual(etfs[i].askPx,etfs[i].lastValidAskPx)) &&  (!areEqual(etfs[i].bidPx,etfs[i].lastValidBidPx)))) { // send orders
            outputQty = 10; // by design the qty for orders is always 10 when they are being sent
            
            // first - cancel previous orders if they exist (their px != 0)
            if ((etfs[i].lastValidAskPx > 0) && (etfs[i].lastValidBidPx > 0)) {
                cout << "BO " << etfs[i].name;
                cout << " " << setprecision (2) << fixed << etfs[i].lastValidBidPx;
                cout << " " << setprecision (0) << fixed << 0 << endl; // etf.bidQty
           
                cout << "AO " << etfs[i].name;
                cout << " " << setprecision (2) << fixed << etfs[i].lastValidAskPx;
                cout << " " << setprecision (0) << fixed << 0 << endl; // etf.askQty
            }
            
            // send bid quote
            cout << "BO " << etfs[i].name;
            cout << " " << setprecision (2) << fixed << etfs[i].bidPx;
            cout << " " << setprecision (0) << fixed << outputQty << endl; // etf.bidQty

            // send ask quote
            cout << "AO " << etfs[i].name;
            cout << " " << setprecision (2) << fixed << etfs[i].askPx;
            cout << " " << setprecision (0) << fixed << outputQty << endl; // etf.askQty
            
            // update previously sent quotes stored in the etf object with new quotes which were sent
            etfs[i].lastValidBidPx = etfs[i].bidPx;
            etfs[i].lastValidAskPx = etfs[i].askPx;
        }

        // if the ETF quote was received from the market, check if we can give a better quote and it will not get the spread crossed, and send such orders
        if ((ask.count(etfs[i].name) > 0) && (bid.count(etfs[i].name) > 0) && (ask.find(etfs[i].name)->second.px - priceTick > bid.find(etfs[i].name)->second.px + priceTick) && (pxNotZero)) {
            outputQty = 10;
            
            // send bid quote
            cout << "BO " << etfs[i].name;
            cout << " " << setprecision (2) << fixed << bid.find(etfs[i].name)->second.px + 0.01;
            cout << " " << setprecision (0) << fixed << outputQty << endl; // etf.bidQty

            // send ask quote
            cout << "AO " << etfs[i].name;
            cout << " " << setprecision (2) << fixed << ask.find(etfs[i].name)->second.px - 0.01;
            cout << " " << setprecision (0) << fixed << outputQty << endl; // etf.askQty
        }
    }
}

int main()
{
    string NumberOfETF_temp;
    getline(cin, NumberOfETF_temp);

    int NumberOfETF = stoi(ltrim(rtrim(NumberOfETF_temp)));

    vector<ETF> etfs; // vector for storing all etfs' objects with their data
    map<string,Message> bid, ask, currencyRate; // map instrument's name to message containing its bid or ask prices or currency rate
    
    for (int NumberOfETF_itr = 0; NumberOfETF_itr < NumberOfETF; NumberOfETF_itr++) {
        string ETFSymbol;
        getline(cin, ETFSymbol);
        
        string ETFCurrency;
        getline(cin, ETFCurrency);

        string NumberOfListingInETF_temp;
        getline(cin, NumberOfListingInETF_temp);

        int NumberOfListingInETF = stoi(ltrim(rtrim(NumberOfListingInETF_temp)));

        vector<vector<string>> listings(NumberOfListingInETF);
        vector<ETFComponent> ETFComponents; // vector for storing components' data for each ETF

        for (int i = 0; i < NumberOfListingInETF; i++) {
            listings[i].resize(3);

            string listings_row_temp_temp;
            getline(cin, listings_row_temp_temp);

            vector<string> listings_row_temp = split(rtrim(listings_row_temp_temp));

            for (int j = 0; j < 3; j++) {
                string listings_row_item = listings_row_temp[j];

                listings[i][j] = listings_row_item;
            }
            // collect ETF's components
            ETFComponents.push_back (ETFComponent(listings[i]));
        }

        // collect ETF's object
        ETF etf = ETF (ETFSymbol, ETFCurrency, ETFComponents);
        etfs.push_back (etf);
    }

    string NumberOfMessages_temp;
    getline(cin, NumberOfMessages_temp);

    int NumberOfMessages = stoi(ltrim(rtrim(NumberOfMessages_temp)));

    vector<vector<string>> messages(NumberOfMessages);

    for (int i = 0; i < NumberOfMessages; i++) {
        messages[i].resize(5);

        string messages_row_temp_temp;
        getline(cin, messages_row_temp_temp);

        vector<string> messages_row_temp = split(rtrim(messages_row_temp_temp));

        for (int j = 0; j < 5; j++) {
            string messages_row_item = messages_row_temp[j];

            messages[i][j] = messages_row_item;
        }
        
        // update the info about prices using the latest message's information
        collectMessageInfo(messages[i], bid, ask, currencyRate);
        
        // try to send out orders (if all necessary conditions are satisfied - playback checks for them)
        playback(etfs, bid, ask, currencyRate);
    }    
    
    // playback(etfs, bid, ask, currencyRate);

    return 0;
}

string ltrim(const string &str) {
    string s(str);

    s.erase(
        s.begin(),
        find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace)))
    );

    return s;
}

string rtrim(const string &str) {
    string s(str);

    s.erase(
        find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(),
        s.end()
    );

    return s;
}

vector<string> split(const string &str) {
    vector<string> tokens;

    string::size_type start = 0;
    string::size_type end = 0;

    while ((end = str.find(" ", start)) != string::npos) {
        tokens.push_back(str.substr(start, end - start));

        start = end + 1;
    }

    tokens.push_back(str.substr(start));

    return tokens;
}

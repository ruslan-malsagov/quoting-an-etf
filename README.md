# quoting-an-etf
My solution to the Quoting an ETF Hackathon by Flow Traders which took place on hackerrank during May 30-31, 2020.

### Description of the problem.
Can be accessed as pdf here (TODO: add a link).

### Description of classes in this application.
Three classes are declared to process all data more conviniently.
Below are class names and names of their key memebers (containing the most important data) in pseudocode:
1) ETFComponent - contains the info about each component in ETF:
    - string symbol
    - string currency
    - double weight
2) ETF - contains the info about the whole ETF:
    - string name
    - string currency
    - vector<ETFComponent> components
3) Message - contains the info about a message with price which are being received from the exchange:
    - string type ("A" for ask, "B" for bid or "X" for currency exchange rate)
    - string symbol (could be currency, ETF or stock symbol)
    - string exchange (exchange name such as stock, etf or currency)
    - double px (price)
    - double qty (quantity) - if 0, then the price of this instrument is not available, but for type == "X" its always zero (by the competition's design)

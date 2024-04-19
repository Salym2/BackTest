#include <iostream>
#include "backtest.h"
#include <fstream>
#include <sstream>

int main() {
    Param para = {1, -1, 0, 0};
    string file = "/Users/fangyuzhe/Desktop/MyBackTest/BackTest/data/IH.csv";
    vector<string> symbols = {"IH"};
    vector<int> dates = {20231009, 20231010, 20231011, 20231012, 20231013, 20231016, 20231017, 20231018, 20231019, 20231020, 20231023, 20231024, 20231025, 20231026, 20231027, 20231030, 20231031, 20231101};
    backtest bt;
    bt.load_data(file);
    bt.acc_run(symbols, dates, para);
    bt.get_order_info({"IH"},{20231020, 20231023, 20231024});
    bt.get_log({"IH"},{20231020, 20231023, 20231024});
    
    return 0;
}

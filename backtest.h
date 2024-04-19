//
//  backtest.h
//  BackTest
//
//  Created by 方宇哲 on 9/4/2024.
//

#ifndef backtest_h
#define backtest_h
#include <vector>
#include <string>
#include <map>
#include <unordered_map>

using namespace std;

struct TradeData {
    string symbol;
    string ID;
    int date;
    int ls; // 0 is long, 1 is short
    long enter_time;
    double enter_price;
    int order_volume;
    int volume;
    long exit_time;
    double exit_price;
    float pnl;
    float HPT;
    float bps;
};

struct Param{
    float open_long;
    float open_short;
    float close_long;
    float close_short;
};

struct Result{
    unordered_map<string, TradeData> order_info;
    vector<string> logs;
    tuple<int, string> key;
};

struct Row{
    int date;
    int time;
    string symbol;
    double askPx1;
    double askPx2;
    double askPx3;
    double bidPx1;
    double bidPx2;
    double bidPx3;
    int askVlm1;
    int askVlm2;
    int askVlm3;
    int bidVlm1;
    int bidVlm2;
    int bidVlm3;
    double lastPx;
    double score;
    double flag;
};

// 定义哈希函数的特化
namespace std {
    template<>
    struct hash<std::tuple<int, std::string>> {
        size_t operator()(const std::tuple<int, std::string>& t) const {
            // 使用组合哈希函数来计算哈希值
            size_t hashInt = std::hash<int>{}(std::get<0>(t));
            size_t hashString = std::hash<std::string>{}(std::get<1>(t));
            return hashInt ^ (hashString << 1);
        }
    };
}

struct Commission{
    int contract_multiplier;
    int type; //0 for percent, 1 for fix
    float open;
    float close;
};

struct CurrentData{
    vector<string> open_orders;
    unordered_map<string, TradeData> order_info;
    vector<string> logs;
    Param param;
    int current_date;
    string current_symbol;
    vector<Row> current_data;
    int current_index;
    int used_id = -1;
    Commission current_commission;
    vector<string> id_list;
};

struct current_statistics{
    float sum_pnl;
    int volume;
    float sum_bps;
    int long_order_count;
    int short_order_count;
    int long_order_sum;
    int short_order_sum;
    float profit_sum;
    int profit_count;
    float loss_sum;
    int loss_count;
    int HPT_sum;
    vector<float> accumulated_pnl;
};

struct Configs{
    unordered_map<string, Commission> commission;
    bool if_broker;
};

class backtest{
private:
    unordered_map<tuple<int, string>, vector<Row>* > datas;
    map<string, int> columns;
    map<tuple<int, string>, vector<TradeData>> total_orders;
    unordered_map<tuple<int, string>, vector<string>> total_logs;
    unordered_map<tuple<int, string>, current_statistics> total_metrics;
    vector<string> targets;
    vector<int> dates;
    Configs config;
    map<string, long> running_times;
    
public:
    backtest(){
        config.commission["IH"] = {300, 0, 0.000023, 0.000023};
        config.if_broker = true;
    }
    ~backtest(){
        for (string symbol: targets){
            for (int date: dates){
                delete datas[{date, symbol}];
            }
        }
    }
    void load_data(const string& filename);
    vector<Row>* get_data(string symbol, int date);
    void base_run(string symbol, int date, Param param);
    void run(vector<string> symbols, vector<int> dates, Param param);
    void next(vector<Row> & data, int index, CurrentData & cd);
    void buy(int volume, float price, CurrentData & cd);
    void sell(int volume, float price, CurrentData & cd);
    void close(string order_id, float price, CurrentData & cd);
    float pnl_calculator(float enter_price, float exit_price, int volume, int ls, CurrentData& cd);
    void get_order_info(vector<string> symbols, vector<int> dates);
    string id_mapping(int ID);
    void check_data(string symbol, int volume);
    void print_row(const Row& row);
    void get_log(vector<string> symbols, vector<int> dates);
    void get_targets();
    void get_dates();
    void acc_run(vector<string> symbols, vector<int> dates, Param param);
};

#endif /* backtest_h */

//
//  backtest.cpp
//  BackTest
//
//  Created by 方宇哲 on 9/4/2024.
//

#include <stdio.h>
#include "backtest.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <thread>
#include <functional>
#include <chrono>


using namespace std;

void backtest::load_data(const string& filename){
    auto start = std::chrono::high_resolution_clock::now();
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Failed to open file: " << filename << endl;
    }

    string line;
    bool flag = true;
    while (getline(file, line)) {
        if (flag){
            string cell;
            istringstream lineStream(line);
            int count = 0;
            while (getline(lineStream, cell, ',')) {
                columns[cell] = count;
                count++;
            }
            flag = false;
        } else{
            Row row;
            istringstream iss(line);
            string token;
            int count = 0;
            while (getline(iss, token, ',')) {
                switch (count) {
                    case 0:
                        row.date = stoi(token);
                        break;
                    case 1:
                        row.time = stoi(token);
                        break;
                    case 2:
                        row.symbol = token;
                        break;
                    case 3:
                        row.askPx1 = stod(token);
                        break;
                    case 4:
                        row.askPx2 = stod(token);
                        break;
                    case 5:
                        row.askPx3 = stod(token);
                        break;
                    case 6:
                        row.bidPx1 = stod(token);
                        break;
                    case 7:
                        row.bidPx2 = stod(token);
                        break;
                    case 8:
                        row.bidPx3 = stod(token);
                        break;
                    case 9:
                        row.askVlm1 = stoi(token);
                        break;
                    case 10:
                        row.askVlm2 = stoi(token);
                        break;
                    case 11:
                        row.askVlm3 = stoi(token);
                        break;
                    case 12:
                        row.bidVlm1 = stoi(token);
                        break;
                    case 13:
                        row.bidVlm2 = stoi(token);
                        break;
                    case 14:
                        row.bidVlm3 = stoi(token);
                        break;
                    case 15:
                        row.lastPx = stod(token);
                        break;
                    case 16:
                        row.score = stod(token);
                        break;
                    case 17:
                        row.flag = stoi(token);
                        break;
                    default:
                        std::cerr << "Error: too many fields" << std::endl;
                        break;
                    }
                    count++;
                }
            vector<Row>* data = get_data(row.symbol, row.date);
            //cout << "111" << endl;
            data->push_back(row);
            
        }
    }
    file.close();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    running_times["load_data"] = duration.count();
}

vector<Row>* backtest::get_data(string symbol, int date){
    tuple<int, string> key = make_tuple(date, symbol);
    auto it = datas.find(key);
    if (it != datas.end()){
        return it->second;
    } else{
        auto t1 = find(targets.begin(), targets.end(), symbol);
        auto t2 = find(dates.begin(), dates.end(), date);
        if (t1 == targets.end())
            targets.push_back(symbol);
        if (t2 == dates.end())
            dates.push_back(date);
        datas[key] = new vector<Row>();
        return datas[key];
    }
}

string backtest::id_mapping(int ID){
    string id = to_string(ID);
    int zerosToAdd = static_cast<int>(8 - id.length());
    return id.insert(0, zerosToAdd, '0');
}


bool compareByString(const TradeData& a, const TradeData& b) {
    return a.ID < b.ID;
}


void backtest::next(vector<Row> & data, int index, CurrentData & cd){
    
    float score = data[index].score;
    
    for (string order_id: cd.open_orders){
        if (cd.order_info[order_id].ls == 0 && score < cd.param.close_long){
            close(order_id, data[index].bidPx1,cd);
        } else if (cd.order_info[order_id].ls == 1 && score > cd.param.close_short){
            close(order_id, data[index].askPx1,cd);
        }
    }
    
    if (score > cd.param.open_long && data[index].askPx1 != 0){
        buy(2, data[index].askPx1,cd);
    } else if (score < cd.param.open_short && data[index].bidPx1 != 0){
        sell(2, data[index].bidPx1,cd);
    }
}


void backtest::buy(int volume, float price, CurrentData & cd){
    cd.used_id++;
    string ID = id_mapping(cd.used_id);
    cd.id_list.push_back(ID);
    TradeData new_record = {cd.current_symbol, ID, cd.current_date, 0, cd.current_data[cd.current_index].time, price, volume, volume, -100, -100, -100, -100, -100};
    
    cd.order_info[ID] = new_record;
    cd.open_orders.push_back(ID);
    
    stringstream ss;
    ss << setw(6) << " Buy "
        << setw(6) << " ID: "
        << setw(6) << ID
        << setw(6) <<  " Symbol: "
        << setw(6) << cd.current_symbol
        << setw(8) << " Time: "
        << setw(8) << cd.current_data[cd.current_index].time
        << setw(6) << " Price: "
        << setw(6) << price
        << setw(6) << " Open Orders:  ";
    for (string id : cd.open_orders){
        ss << id << " ";
    }
    ss << endl;
    cd.logs.push_back(ss.str());
    
}

void backtest::sell(int volume, float price, CurrentData& cd){
    cd.used_id++;
    string ID = id_mapping(cd.used_id);
    cd.id_list.push_back(ID);
    TradeData new_record = {cd.current_symbol, ID, cd.current_date, 1, cd.current_data[cd.current_index].time, price, volume, volume, -100, -100, -100, -100, -100};
    
    cd.order_info[ID] = new_record;
    cd.open_orders.push_back(ID);
    
    stringstream ss;
    ss << setw(6) << "Sell "
        << setw(6) << " ID: "
        << setw(6) << ID
        << setw(6) <<  " Symbol: "
        << setw(6) << cd.current_symbol
        << setw(8) << " Time: "
        << setw(8) << cd.current_data[cd.current_index].time
        << setw(6) << " Price: "
        << setw(6) << price
        << setw(6) << " Open Orders:  ";
    for (string id : cd.open_orders){
        ss << id << " ";
    }
    ss << endl;
    cd.logs.push_back(ss.str());
}


void backtest::close(string order_id, float price, CurrentData& cd){
    
    auto it = std::find(cd.open_orders.begin(), cd.open_orders.end(), order_id);
    if (it != cd.open_orders.end()) {
        cd.open_orders.erase(it);
    } else{
        return;
    }
    int& time = cd.current_data[cd.current_index].time;
    cd.order_info[order_id].exit_price = price;
    cd.order_info[order_id].exit_time = time;
    cd.order_info[order_id].pnl = pnl_calculator(cd.order_info[order_id].enter_price, price, cd.order_info[order_id].volume, cd.order_info[order_id].ls, cd);
    cd.order_info[order_id].HPT = (time - cd.order_info[order_id].enter_time)/1000;
    cd.order_info[order_id].bps = (cd.order_info[order_id].exit_price / cd.order_info[order_id].enter_price - 1) * 1e4;
    
    stringstream ss;
    ss << setw(6) << "Close "
        << setw(6) << " ID: "
        << setw(6) << order_id
        << setw(6) <<  " Symbol: "
        << setw(6) << cd.current_symbol
        << setw(8) << " Time: "
        << setw(8) << cd.current_data[cd.current_index].time
        << setw(6) << " Price: "
        << setw(6) << price
        << setw(6) << " Open Orders:  ";
    
    for (string id : cd.open_orders){
        ss << id << " ";
    }
    ss << endl;
    cd.logs.push_back(ss.str());
    
}


float backtest::pnl_calculator(float enter_price, float exit_price, int volume, int ls, CurrentData& cd){
    float commission;
    if (cd.current_commission.type == 0){
        commission = (enter_price * cd.current_commission.open + exit_price * cd.current_commission.close) * volume;
    } else{
        commission = (cd.current_commission.open + cd.current_commission.close) * volume;
    }
    float pnl = (exit_price - enter_price) * cd.current_commission.contract_multiplier * volume * pow(-1, ls) - commission;
    return pnl;
}


void backtest::base_run(string symbol, int date, Param param){
    CurrentData cd;
    vector<Row> data = *datas[{date, symbol}];
    cd.current_data = data;
    cd.current_date = date;
    cd.current_symbol = symbol;
    cd.used_id = -1;
    cd.current_commission = config.commission[symbol];
    cd.param = param;
    for (int i = 0; i < data.size(); i++){
        cd.current_index = i;
        next(data, i, cd);
    }
    
    vector<string> temp_open_orders = cd.open_orders;
    
    if (temp_open_orders.size() > 0){
        for (string order_id: temp_open_orders){
        
            if (cd.order_info[order_id].ls == 0){
                close(order_id, data[data.size()-1].bidPx1, cd);
            } else {
                close(order_id, data[data.size()-1].askPx1, cd);
            }
        }
    }
    vector<TradeData> vec;
    for (string id: cd.id_list){
        vec.push_back(cd.order_info[id]);
    }
    total_orders[{date,symbol}] = vec;
    total_logs[{date,symbol}] = cd.logs;
};


void backtest::run(vector<string> symbols, vector<int> dates, Param param){
    auto start = std::chrono::high_resolution_clock::now();

    for (string symbol: symbols){
        for (int date: dates){
            base_run(symbol, date, param);
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    running_times["run"] = duration.count();
}

void backtest::acc_run(vector<string> symbols, vector<int> dates, Param param){
    auto start = std::chrono::high_resolution_clock::now();
    vector<thread> threads;
    for (string symbol: symbols){
        for (int date: dates){
            threads.emplace_back(bind(&backtest::base_run, this, symbol, date, param));
        }
    }
    for (auto& thread : threads) {
        thread.join();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    running_times["acc_run"] = duration.count();
}




void backtest::print_row(const Row& row){
    std::cout << std::setw(10) << row.date << " "
              << std::setw(10) << row.time << " "
              << std::setw(10) << row.symbol << " "
              << std::setw(10) << row.askPx1 << " "
              << std::setw(10) << row.askPx2 << " "
              << std::setw(10) << row.askPx3 << " "
              << std::setw(10) << row.bidPx1 << " "
              << std::setw(10) << row.bidPx2 << " "
              << std::setw(10) << row.bidPx3 << " "
              << std::setw(10) << row.askVlm1 << " "
              << std::setw(10) << row.askVlm2 << " "
              << std::setw(10) << row.askVlm3 << " "
              << std::setw(10) << row.bidVlm1 << " "
              << std::setw(10) << row.bidVlm2 << " "
              << std::setw(10) << row.bidVlm3 << " "
              << std::setw(10) << row.lastPx << " "
              << std::setw(10) << row.score << " "
              << std::setw(10) << row.flag << std::endl;
}




void backtest::check_data(string symbol, int volume){
    
    std::cout << std::setw(10) << "date" << " "
              << std::setw(10) << "time" << " "
              << std::setw(10) << "symbol" << " "
              << std::setw(10) << "askPx1" << " "
              << std::setw(10) << "askPx2" << " "
              << std::setw(10) << "askPx3" << " "
              << std::setw(10) << "bidPx1" << " "
              << std::setw(10) << "bidPx2" << " "
              << std::setw(10) << "bidPx3" << " "
              << std::setw(10) << "askVlm1" << " "
              << std::setw(10) << "askVlm2" << " "
              << std::setw(10) << "askVlm3" << " "
              << std::setw(10) << "bidVlm1" << " "
              << std::setw(10) << "bidVlm2" << " "
              << std::setw(10) << "bidVlm3" << " "
              << std::setw(10) << "lastPx" << " "
              << std::setw(10) << "score" << " "
              << std::setw(10) << "flag" << std::endl;
    
    vector<Row> temp = *datas[{volume, symbol}];
    for (const auto & row : temp){
        print_row(row);
    }
}



void backtest::get_order_info(vector<string> symbols, vector<int> dates) {
    auto start = std::chrono::high_resolution_clock::now();
    cout << setw(10) << "Date"
        << setw(10) << "Symbol"
        << setw(10) << "ID"
        << setw(12) << "Enter_Time"
        << setw(12) << "Enter_Price"
        << setw(14) << "Order_Volume"
        << setw(10) << "Volume"
        << setw(12) << "Exit_Time"
        << setw(12) << "Exit_Price"
        << setw(8)  << "LS"
        << setw(10) << "PNL"
        << setw(10) << "HPT"
        << setw(14) << "BPS" << "\n";
        
    for (string symbol : symbols){
        for (int date: dates){
            vector<TradeData> oneday_order = total_orders[{date, symbol}];
            for (const TradeData & order : oneday_order){
                if (order.date == 0) continue;
                cout << setw(10) << order.date
                    << setw(10) << order.symbol
                    << setw(10) << order.ID
                    << setw(12) << order.enter_time
                    << setw(12) << order.enter_price
                    << setw(14) << order.order_volume
                    << setw(10) << order.volume
                    << setw(12) << order.exit_time
                    << setw(12) << order.exit_price
                    << setw(8) << order.ls
                    << setw(10) << fixed << setprecision(2) << order.pnl
                    << setw(10) << fixed << order.HPT
                    << setw(14) << fixed << setprecision(2) << order.bps
                    << endl;
            }
        }
    }
    cout << endl;
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    running_times["get_order_info"] = duration.count();
}



void backtest::get_log(vector<string> symbols, vector<int> dates){
    for (string symbol : symbols){
        for (int date: dates){
            for (string log : total_logs[{date, symbol}]){
                cout << log;
            }
        }
    }
    cout << endl;
}

void backtest::get_targets(){
    int n = 0;
    for (string symbol : targets){
        n++;
        if (n != targets.size())
            cout << symbol << ", ";
        else
            cout << symbol << endl;
    }
}

void backtest::get_dates(){
    int n = 0;
    for (int date : dates){
        n++;
        if (n != dates.size())
            cout << date << ", ";
        else
            cout << date << endl;
    }
}


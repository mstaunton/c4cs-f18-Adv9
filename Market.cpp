#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <sstream>
#include <getopt.h>
#include <iomanip>
#include "P2random.h"
#include <queue>

using namespace std;

struct Order{
	char type = 'p';
	long long order_num = -1;
	int trader_num = -1;
	int stock_num = -1;
	int price = -3;
	int quantity = -1;
	int time_stamp = -1;
};



struct Order_Buy_Comparator{
	bool operator() (Order& o1, Order& o2){
		if(o1.price < o2.price){
			return true;
		}
		else if(o1.price == o2.price){
			if(o1.order_num > o2.order_num){
				return true;
			}
			else{
				return false;
			}
		}
		else{
			return false;
		}
	}
};

struct Order_Sell_Comparator{
	bool operator() (Order& o1, Order& o2){
		if(o1.price > o2.price){
			return true;
		}
		else if(o1.price == o2.price){
			if(o1.order_num > o2.order_num){
				return true;
			}
			else{
				return false;
			}
		}
		else{
			return false;
		}
	}
};

struct Time_Traveler{
	int CBP = -1;
	int PBP = -1;
	int CBT = -1;
	int PBT = -1;
	int CSP = -1;
	int CST = -1;
};

struct Stock{
	int med_diff = 0;
	priority_queue<Order, vector<Order>, Order_Buy_Comparator> buy_queue;
	priority_queue<Order, vector<Order>, Order_Sell_Comparator> sell_queue;
	priority_queue<int, vector<int>, greater<int> > larger;
	priority_queue<int> lesser;
	Time_Traveler time_travel;
};

struct Transaction{
	int buyer_ID = -1;
	int seller_ID = -1;
	int stock_num = -1;
	int quantity = -1;
	int price = -2;
};

struct Trader_Info{
	int bought = 0;
	int sold = 0;
	long long profit = 0;
};
bool will_purchase_occur(Stock &current_stock, Order &current_order){
	if(current_stock.sell_queue.empty()){
		return false;
	}
	Order lowest = current_stock.sell_queue.top();
	if(lowest.price <= current_order.price){
		return true;
	}
	else{
		return false;
	}
}

bool will_sale_occur(Stock &current_stock, Order &current_order){
	if(current_stock.buy_queue.empty()){
		return false;
	}
	Order greatest = current_stock.buy_queue.top();
	if(greatest.price >= current_order.price){
		return true;
	}
	else{
		return false;
	}
}

Transaction buy_transaction(Stock &current_stock, Order &current_order){
	Transaction result;
	Order lowest = current_stock.sell_queue.top();
	result.buyer_ID = current_order.trader_num;
	result.seller_ID = lowest.trader_num;
	result.stock_num = current_order.stock_num;
	result.price = lowest.price;
	current_stock.sell_queue.pop();
	if(current_order.quantity > lowest.quantity){
		current_order.quantity -= lowest.quantity;
		result.quantity = lowest.quantity;
	}
	else if(current_order.quantity < lowest.quantity){
		result.quantity = current_order.quantity;
		lowest.quantity -= current_order.quantity;
		current_order.quantity = 0;
		current_stock.sell_queue.push(lowest);
	}
	else{
		result.quantity = current_order.quantity;
		current_order.quantity = 0;
	}
	return result;
}

Transaction sell_transaction(Stock &current_stock, Order &current_order){
	Transaction result;
	Order greatest = current_stock.buy_queue.top();
	result.buyer_ID = greatest.trader_num;
	result.seller_ID = current_order.trader_num;
	result.stock_num = current_order.stock_num;
	result.price = greatest.price;
	current_stock.buy_queue.pop();
	if(current_order.quantity > greatest.quantity){
		current_order.quantity -= greatest.quantity;
		result.quantity = greatest.quantity;
	}
	else if(current_order.quantity < greatest.quantity){
		result.quantity = current_order.quantity;
		greatest.quantity -= current_order.quantity;
		current_order.quantity = 0;
		current_stock.buy_queue.push(greatest);
	}
	else{
		result.quantity = current_order.quantity;
		current_order.quantity = 0;
	}
	return result;
}
void add_transaction(Stock &current_stock, int price){ // adds price to median priority queues
	if(current_stock.lesser.empty()){ // if lesser is empty, push to lesser
		current_stock.lesser.push(price);
		current_stock.med_diff = 1;
	}
	else{
		if(price < current_stock.lesser.top()){ // if price is less than lesser, push to lesser
			current_stock.lesser.push(price);
			++current_stock.med_diff;
		}
		else{ // push to top
			current_stock.larger.push(price);
			--current_stock.med_diff;
		}
	}
}

void balance_p_queues(Stock &current_stock){
	if(current_stock.med_diff == 2){ // if lesser too big, push top of lesser to larger
		current_stock.larger.push(current_stock.lesser.top());
		current_stock.lesser.pop();
		current_stock.med_diff = 0; // PQ are balanced, index is middle
	}
	else if(current_stock.med_diff == -2){ // if larger too big, push top of larger to lesser
		current_stock.lesser.push(current_stock.larger.top());
		current_stock.larger.pop();
		current_stock.med_diff = 0; // PQ are balanced, index is middle
	}
	else{
		return; // 1 is top of lesser, 0 is average of top of both, -1 is top of larger
	}
}

long long median(Stock &current_stock){
	if(current_stock.med_diff == -1){
		return (static_cast<long long>(current_stock.larger.top()));
	}
	else if(current_stock.med_diff == 1){
		return (static_cast<long long>(current_stock.lesser.top()));
	}
	else{
		return (static_cast<long long>((current_stock.larger.top() + current_stock.lesser.top() ) / 2));
	}
}

void get_median_index(Stock &current_stock, int price){
	add_transaction(current_stock, price);
	balance_p_queues(current_stock);
}

void print_median(Stock &current_stock, int time_stamp, int index){
	if(!(current_stock.lesser.empty())){
		cout << "Median match price of Stock " << index << " at time " << time_stamp << " is $" << median(current_stock) << "\n";
	}
}

void time_traveler_func(Time_Traveler &t, Order &current_order){
	if(current_order.type == 's'){
		if(t.CBP == -1){
			t.CBP = current_order.price;
			t.CBT = current_order.time_stamp;
		}
		else{
			if(current_order.price < t.CBP){
				if(t.CSP == -1){
					t.CBP = current_order.price;
					t.CBT = current_order.time_stamp;
				}
				else if(t.PBP == -1){
					t.PBP = current_order.price;
					t.PBT = current_order.time_stamp;
				}
				else{
					if(current_order.price < t.PBP){
						t.PBP = current_order.price;
						t.PBT = current_order.time_stamp;
					}
					else{
						return;
					}
				}
			}
		}
	}
	else{
		if(t.CBP == -1){
			return;
		}
		else{
			if(current_order.price <= t.CBP){
				return;
			}
			else{
				if(t.CSP == -1){
					t.CSP = current_order.price;
					t.CST = current_order.time_stamp;
				}
				else if(t.PBP == -1){
					if(current_order.price > t.CSP){
						t.CSP = current_order.price;
						t.CST = current_order.time_stamp;
					}
					else{
						return;
					}
				}
				else{
					if((current_order.price - t.PBP) > (t.CSP - t.CBP)){
						t.CSP = current_order.price;
						t.CST = current_order.time_stamp;
						t.CBP = t.PBP;
						t.CBT = t.PBT;
					}
				}
			}
		}
	}
}

int main(int argc, char *argv[]) {
    std::ios_base::sync_with_stdio(false);
    
    int gotopt;
    int option_index = 0;
    
    option long_opts[] = {
    {"verbose", no_argument, nullptr, 'v'},
    {"median", no_argument, nullptr, 'm'},
    {"trader_info", no_argument, nullptr, 'i'},
    {"time_travelers", no_argument, nullptr, 't'},
    {nullptr, 0, nullptr, '\0'},
	};

	bool verbose_was_set = false;
	bool median_was_set = false;
	bool trader_info_was_set = false;
	bool time_travelers_was_set = false;

	while ((gotopt = getopt_long(argc, argv, "vmit", long_opts, &option_index)) != -1) {
		switch(gotopt) {
			case 'v':
				verbose_was_set = true;
				break;
			case 'm':
				median_was_set = true;
				break;
			case 'i':
				trader_info_was_set = true;
				break;
			case 't':
				time_travelers_was_set = true;
				break;
			default:
				cerr << "Error: Invalid command line \n";
				exit(1);
		}
	}

	string comment = "";
	getline(cin, comment);// takes in Comment line

	string description = "";
	string input_mode = "";
	bool PR_mode = false;
	cin >> description >> input_mode;
	if(input_mode == "TL"){
		PR_mode = false;
	}
	else if(input_mode == "PR"){
		PR_mode = true;
	}
	else{
		cerr << "Error: Invalid Input Mode \n";
		exit(1);
	}

	unsigned int num_traders;
	unsigned int num_stocks;
	cin >> description >> num_traders >> description >> num_stocks;

	unsigned int seed;
	unsigned int num_orders;
	unsigned int arrival_rate;

	stringstream ss;

	if(PR_mode){
		cin >> description >> seed >> description >> num_orders >> description >> arrival_rate;
		P2random::PR_init(ss, seed, num_traders, num_stocks, num_orders, arrival_rate);
	}

	istream & inputStream = PR_mode ? ss : cin;

	Order current_order;

	int last_time_stamp = -1;

	int current_time_stamp = -1;
	char junk;
	string type = "";
	unsigned int trader_num;
	unsigned int stock_num;
	int stock_price;
	int stock_quantity;


	vector<Trader_Info> traders;
	traders.resize(num_traders);

	vector<Stock> market;
	market.resize(num_stocks);

	cout << "Processing orders...\n";

	long long num_transactions = 0;
	long long order_num = 0;

	while(inputStream >> current_time_stamp >> type >>junk >> trader_num >> junk >> stock_num >> junk >> stock_price >> junk >> stock_quantity){
		if(current_time_stamp < 0){
			cerr << "Error: Time Stamp is negative\n";
			exit(1);
		}
		if(current_time_stamp < last_time_stamp){
			cerr << "Error: Time Stamps not decreasing \n";
			exit(1);
		}
		++order_num;
		current_order.order_num = order_num;
		current_order.time_stamp = current_time_stamp;
		if(type == "BUY"){
			current_order.type = 'b';
		}
		else if(type == "SELL"){
			current_order.type = 's';
		}
		else{
			cerr << "Error: Invalid type argument \n";
			exit(1);
		}
		if(trader_num >= num_traders){
			cerr << "Error: Invalid Trader_ID\n";
			exit(1);
		}
		current_order.trader_num = trader_num;

		if(stock_num >= num_stocks){
			cerr << "Error: Invalid Stock Number\n";
			exit(1);
		}
		current_order.stock_num = stock_num;

		if(stock_price < 1){
			cerr << "Error: Invalid Price\n";
			exit(1);
		}
		current_order.price = stock_price;

		if(stock_quantity < 1){
			cerr << "Error: Invalid Quantity\n";
			exit(1);
		}
		current_order.quantity = stock_quantity;

		if(time_travelers_was_set){
			time_traveler_func(market[current_order.stock_num].time_travel, current_order);
		}

		if(median_was_set){
			if(num_transactions > 0){
				if(current_time_stamp > last_time_stamp){
					for(unsigned int i = 0; i < num_stocks; ++i){
						print_median(market[i], last_time_stamp, i);
					}
				}
			}
		}
		last_time_stamp = current_time_stamp;
		Transaction result;
		while(current_order.quantity > 0){
			if(current_order.type == 'b'){
				if(will_purchase_occur(market[current_order.stock_num], current_order)){
					result = buy_transaction(market[current_order.stock_num], current_order);
					++num_transactions;
					if(trader_info_was_set){
						traders[result.buyer_ID].bought += result.quantity;
						traders[result.buyer_ID].profit -= (static_cast<long long>(result.quantity) * static_cast<long long>(result.price));
						traders[result.seller_ID].sold  += result.quantity;
						traders[result.seller_ID].profit += (static_cast<long long>(result.quantity) * static_cast<long long>(result.price));
					}
					if(median_was_set){
						get_median_index(market[current_order.stock_num], result.price);
					}
				}
				else{
					market[current_order.stock_num].buy_queue.push(current_order);
					break;
				}
			}
			else{
				if(will_sale_occur(market[current_order.stock_num], current_order)){
					result = sell_transaction(market[current_order.stock_num], current_order);
					++num_transactions;
					if(trader_info_was_set){
						traders[result.buyer_ID].bought += result.quantity;
						traders[result.buyer_ID].profit -= (static_cast<long long>(result.quantity) * static_cast<long long>(result.price));
						traders[result.seller_ID].sold  += result.quantity;
						traders[result.seller_ID].profit += (static_cast<long long>(result.quantity) * static_cast<long long>(result.price));
					}
					if(median_was_set){
						get_median_index(market[current_order.stock_num], result.price);
					}
				}
				else{
					market[current_order.stock_num].sell_queue.push(current_order);
					break;
				}
			}
			if(verbose_was_set){
				cout << "Trader " << result.buyer_ID << " purchased " << result.quantity << " shares of Stock " << result.stock_num << " from Trader " << result.seller_ID << " for $" << result.price << "/share\n";
			}
		}
	}
	if(median_was_set){
		for(unsigned int i = 0; i < num_stocks; ++i){
			print_median(market[i], last_time_stamp, i);
		}
	}

	cout << "---End of Day---\n";
	cout << "Orders Processed: " << num_transactions << "\n";

	if(trader_info_was_set){
		cout << "---Trader Info---\n";
		for(unsigned int i = 0; i < num_traders; ++i){
			cout << "Trader " << i << " bought " << traders[i].bought << " and sold " << traders[i].sold << " for a net transfer of $" << traders[i].profit << "\n";
		}
	}

	if(time_travelers_was_set){
		cout << "---Time Travelers---\n";
		for(unsigned int i = 0; i < num_stocks; ++i){
			if(market[i].time_travel.CSP == -1){
				cout << "A time traveler would buy shares of Stock " << i << " at time: -1 and sell these shares at time: -1\n";
			}
			else{
				cout << "A time traveler would buy shares of Stock " << i << " at time: " << market[i].time_travel.CBT << " and sell these shares at time: " << market[i].time_travel.CST << "\n";
			}
		}
	}


	return 0;



}
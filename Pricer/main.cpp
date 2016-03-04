#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

using namespace std;

enum class Options : char { BUY = 'b', IDLE = '_', SELL = 's' };

struct TickOptions {
  bool can_buy;
  bool can_idle;
  bool can_sell;
  Options decision;

  bool decisions_available() const { return can_buy || can_idle || can_sell; }
};

class TickLine {
public:
  TickLine();
  vector<TickOptions> ticks;
  int curr_tick;
  int shares;
  const int first_tick = 0;
  const int last_tick = 99;

  void next_tick();
  void decide();

  bool gen_next_line();

  int calc_profit(vector<int> &price);

  string toString();
};

bool TickLine::gen_next_line() {

  while (curr_tick != -1) {

    auto &t = ticks[curr_tick]; // get current tick

    if (t.decisions_available()) { // can test another decision
      decide();
      if (curr_tick < last_tick) { // if not last tick, continue to populate
        ++curr_tick;
        next_tick();
      } else
        return true; // if last tick, tickline is complete
    } else
      --curr_tick; // no decisions available for current tick, go back a tick
  }
  return false; // all tick combination are tested
}

int TickLine::calc_profit(vector<int> &price) {
  int sum = 0;
  int shares_ = 0;
  auto p = price.begin();

  for (auto t : ticks)
    switch (t.decision) {
    case Options::BUY:
      sum -= *(p++);
      ++shares_;
      break;
    case Options::SELL:
      sum += *(p++);
      --shares_;
      break;
    case Options::IDLE:
      p++;
      break;
    }

  if (shares_) {
    sum += shares_ *
           price.back(); // sell all remaining shares at last tick's price
    shares_ = 0;
  }

  return sum;
}

string TickLine::toString() {
  string s;
  s.reserve(100);

  for (auto tick : ticks)
    s.push_back(static_cast<char>(tick.decision));

  return s;
}

TickLine::TickLine() {
  curr_tick = 0;
  ticks = decltype(ticks)(100);
  next_tick();
  shares = 0;
}

void TickLine::next_tick() {
  TickOptions tick;

  tick.can_buy = true;
  tick.can_idle = true;
  tick.can_sell = (shares > 0);

  ticks[curr_tick] = tick;
}

void TickLine::decide() {
  TickOptions &t = ticks[curr_tick];

  if (t.can_buy) {
    t.decision = Options::BUY;
    t.can_buy = false;
    ++shares;
  } else if (t.can_idle) {
    t.decision = Options::IDLE;
    t.can_idle = false;
  } else if (t.can_sell) {
    t.decision = Options::SELL;
    t.can_sell = false;
    --shares;
  }
}

vector<int> read_data(string fname) {

  ifstream inp_stream;
  inp_stream.open(fname, ios::in);

  if (!inp_stream.is_open()) {
    cerr << "Could not open file!";
    exit(1);
  }

  string line;
  getline(inp_stream, line);

  vector<int> d;
  d.resize(100, 0);

  for (auto &e : d) {
    int num;
    inp_stream >> num;
    inp_stream.get();
    inp_stream >> e;
  }
  return d;
}

string gen_str() { return ""; }

int eval_str(string cs) { return 0; }

int main(int argc, char **argv) {

  if (argc < 2)
    return 1;

  auto data = read_data(argv[1]);

  int max_profit = -50'000;
  int million = 0;
  int profit;
  unsigned long long mils = 0;
  string best_str{100, '0'};

  TickLine t;

  while (t.gen_next_line()) {
    ;
    if (++million % 1'000'000 == 0)
      cout << t.toString() << ' ' << ++mils << "\'M " << max_profit << '\n';
    profit = t.calc_profit(data);
    if (profit > max_profit) {
      max_profit = profit;
      best_str = t.toString();
    }
  }

  cout << max_profit << ' ' << best_str << '\n';

  return 0;
}
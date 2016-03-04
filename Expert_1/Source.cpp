//#include <stdlib.mqh>
////////////////////////////
//extern double BarSize = 20;


//#import "mql4_connect.dll"
//string readStr(string);
//bool writeStr(string, string, int);
//#import


extern int TickCount = 3;
extern string note1 = "Stochastic settings";
extern int       KPeriod = 2;
extern int       Slowing = 1;
extern int       DPeriod = 2;
extern double    Kdop = 3;
extern double    Ddop = 2;
extern string note2 = "0=sma, 1=ema, 2=smma, 3=lwma";
extern int       MAMethod = 2;
extern string note3 = "0=high/low, 1=close/close";
extern int       PriceField = 1;
extern string note4 = "NoLossStart - пункты от начала";
extern int NoLossStart = 50;
extern int NoLossStart2 = 1000;
extern string note5 = "NoLoss - проценты от объема";
extern int NoLoss = 80;
extern int NoLoss2 = 30;
extern string note6 = "StopLoss - пункты";
extern int StopLoss = 200;

extern double Lots = 20;
extern double ExtraLots = 10;
extern bool Risk = true;
extern double BeginTime = -1;
extern double EndTime = -1;
//+maxlots
//+extra order (points)
extern string note7 = "CloseFriday - 0-все остаютс€ 1-все выключаютс€ 2-выключаютс€ дополнительные";
extern int CloseFriday = 0;
extern int MaxLots = 4;
extern double ExtraOrder = 0.05;
extern int ExtraTimeframe = 30;
extern double ExtraStochHigh = 80.0;
extern double ExtraStochLow = 20.0;
extern double TakeProfitHigh = 96.0;
extern double TakeProfitLow = 8.0;
//////////////////////////

bool manual_mode = true;
double Kvar;
double Dvar;
double DvarOld;
double KvarExtra;
double KvarExtraOld;
double KvarOld;
string tmp_str;
int MainTimeFrame;
bool buying = false;
bool selling = false;
int buying_count = 0;
bool buy_later = false;
bool sell_later = false;
double extra_buy_price = 0;
double extra_sell_price = 0;
bool stop_buy_later = false;
bool stop_sell_later = false;
int selling_count = 0;
bool dealing = false;
double deal_price;
double stop_price;
int buy_ticket;
int sell_ticket;
int my_magic_num;
int my_magic_num_extra;

double stat_bar_size;
double bar_size;
bool prev_bar;
bool cur_bar;
datetime cur_time;
datetime tick_time;
datetime prev_time;
int cur_ticks = 0;
int bar_ticks = 0;
datetime bar_time;

bool new_tick = false;
bool new_bar = true;

bool freeze_trading = false;

bool freeze_trailing = false;

bool up_passed;
bool down_passed;
double last_max_rsi;

int cur_slippage;

string cur_symbol;

bool up;

string comment_msg;
string old_comment_msg_1;
string old_comment_msg_2;
string old_comment_msg_3;

double right_direction_amount;
double last_max_price;
double last_price;

double last_max_bid;
double last_min_ask;

bool buy_profit_armed = false;
bool sell_profit_armed = false;

double cur_price()
{
	return ((Ask + Bid) / 2);
}

int tickets[100];
int tmp_tickets[100];
double cur_stops[100];
double open_prices[100];
int order_types[100];
int null_passed[100];


int vect_size = 0;

void add_ticket(int _ticket, double _stop, double _open, int _type, int _null_passed)
{
	tickets[vect_size] = _ticket;
	cur_stops[vect_size] = _stop;
	open_prices[vect_size] = _open;
	order_types[vect_size] = _type;
	null_passed[vect_size] = _null_passed;

	vect_size++;
}

int pop_ticket_ind(int _ind)
{
	int tmp = tickets[_ind];
	vect_size--;
	for (int i = _ind; i<vect_size; i++)
	{
		tickets[i] = tickets[i + 1];
		cur_stops[i] = cur_stops[i + 1];
		open_prices[i] = open_prices[i + 1];
		order_types[i] = order_types[i + 1];
		null_passed[i] = null_passed[i + 1];
	}
	return (tmp);
}

void pop_ticket_val(int _ticket)
{
	for (int i = 0; i<vect_size; i++)
		if (tickets[i] == _ticket)
		{
			pop_ticket_ind(i);
			break;
		}
}

int handler;
bool already_told = false;

string time_stamp()
{
	return (Hour() + ":" + Minute() + ":" + Seconds() + " ");
}

void tell_my_name()
{

	string msg = readStr("out_ilv.csv");
	if (msg == "")
		return;

	if (StringFind(msg, "start_write") == 0)
	{
		if (!already_told)
		{
			if (!writeStr("in_ilv.csv", tmp_str + "\n", 1))
				return;
			already_told = true;
		}
	}

	if (StringFind(msg, "stop_write") == 0)
	{
		already_told = false;
	}


}

void MyAlert(string msg)
{
	if (IsDllsAllowed())
		writeStr(Day() + "." + Month() + "." + Year() + " " + tmp_str + "log.txt", time_stamp() + "Alert " + msg + "\n", 1);
	Alert(msg);
}

void read_command()
{

	/*Print(readStr(tmp_str));
	//writeStr(tmp_str, AccountCompany()+"\n"+AccountServer());*/


	string msg = readStr(tmp_str + ".csv");
	if (msg == "")
		return;

	if (StringFind(msg, "STUB") == 0)
	{
		return;
	}
	Print(msg);
	if (StringFind(msg, "BUY") == 0)
	{
		freeze_trading = false;
		start_buy();
	}
	else
		if (StringFind(msg, "SELL") == 0)
		{
			freeze_trading = false;
			start_sell();
		}
		else
			if (StringFind(msg, "MANUAL_ON") == 0)
			{
				manual_mode = true;
			}
			else
				if (StringFind(msg, "MANUAL_OFF") == 0)
				{
					manual_mode = false;
				}
				else
					if (StringFind(msg, "ADD") == 0)
					{
						double tmp_lot = StrToDouble(StringSubstr(msg, 4));
						double tmp_old_lot = ExtraLots;
						bool old_risk = Risk;
						Risk = false;
						ExtraLots = tmp_lot;
						int tmp_ticket;
						if (buying)
						{
							start_buy(true);
						}
						if (selling)
						{
							start_sell(true);
						}
						ExtraLots = tmp_old_lot;
						Risk = old_risk;
					}
					else
						if (StringFind(msg, "STOP") == 0)
						{
							stop_trading();
							freeze_trading = true;
						}
						else
							if (StringFind(msg, "START") == 0)
							{
								freeze_trading = false;
							}
							else
								if (StringFind(msg, "DIRECTION") == 0)
								{
									if (!buying && !selling)
										return;
									if (buying)
									{
										stop_buy();
										start_sell();
									}
									else
									{
										stop_sell();
										start_buy();
									}
								}

	for (int ii = 0; ii< 3; ii++)
		if (writeStr(tmp_str + ".csv", "STUB", 0))
			return;

	MyAlert("«апись заглушки в фал окончилась неудачей");
}
void start_deal()
{
	dealing = true;
	last_max_price = stop_price;
}
void stop_deal()
{
	dealing = false;
	last_price = cur_price();
}
void start_buy(bool extra = false)
{
	GetLastError();
	double loc_lot;
	if (!extra)
		if (Risk)
			loc_lot = LOT(Lots, 1);
		else
			loc_lot = Lots;
	else
		if (Risk)
			loc_lot = LOT(ExtraLots, 1);
		else
			loc_lot = ExtraLots;


	int count = 0;
	int err;
	int tmp_slippage = cur_slippage;
	while (count<3)
	{
		RefreshRates();
		color tmp_color = Green;
		if (extra)
			tmp_color = Yellow;
		int tmp_magic_num = my_magic_num;
		if (extra)
			tmp_magic_num = my_magic_num_extra;

		double tmp_stoploss = 0;
		if (StopLoss > 0)
			tmp_stoploss = NormalizeDouble(Bid - StopLoss*Point, Digits);

		writeStr(Day() + "." + Month() + "." + Year() + " " + tmp_str + "log.txt", time_stamp() + "start buy lot " + loc_lot + " Ask " + Ask + " Bid " + Bid + "\n", 1);
		buy_ticket = OrderSend(cur_symbol, OP_BUY, loc_lot, MarketInfo(cur_symbol, MODE_ASK), tmp_slippage, tmp_stoploss, 0, "up_down buy", tmp_magic_num, tmp_color);
		err = GetLastError();
		if ((buy_ticket == -1) && (err != 1))
		{
			writeStr(Day() + "." + Month() + "." + Year() + " " + tmp_str + "log.txt", time_stamp() + "start buy error " + err + "\n", 1);
			if (err == 138)
				tmp_slippage += 5;
			if (err == 146)
			{
				while (IsTradeContextBusy())
					Sleep(100);
				continue;
			}
			//Comment("error(",err,"): ",ErrorDescription(err));
			//Print("error(",err,"): ",ErrorDescription(err));
		}
		else
		{
			//         Print("buy_ticket = ", buy_ticket);
			buying = true;
			deal_price = Ask;
			stop_price = Bid;
			extra_buy_price = Bid;
			last_max_bid = Bid;
			add_ticket(buy_ticket, tmp_stoploss, Bid, 0, 0);
			start_deal();
			buying_count++;
			break;
		}
		count++;
	}
	if (count == 3)
		if (err == 130)
			MyAlert("start buy ошибка 130 Ask " + Ask + ", Bid " + Bid + ", tmp_tral " + 0 + ", tp_buy " + 0);
		else
			if (err == 138)
				buy_later = true;
			else
				MyAlert("start buy ошибка с кодом " + err);
}

void stop_buy(bool extra = false)
{
	GetLastError();
	if (!extra)
	{
		buying = false;
		stop_deal();
	}
	//OrderClose(buy_ticket,Lots,Bid,3,Green);
	int tmp_size = vect_size;
	for (int pos = 0;pos<tmp_size; pos++)
	{
		tmp_tickets[pos] = tickets[pos];
	}
	for (pos = 0;pos<tmp_size; pos++)
	{
		if (OrderSelect(tmp_tickets[pos], SELECT_BY_TICKET, MODE_TRADES) == false)
			continue;
		int err = GetLastError();
		/*
		if ((err != 0) && (err != 4099))
		{
		//Comment("error(",err,"): ",ErrorDescription(err));
		//Print("error(",err,"): ",ErrorDescription(err));
		MyAlert("stop buy select ошибка с кодом " + err);
		continue;
		}
		*/
		if (OrderType() == OP_BUY)
		{
			if (extra && (OrderMagicNumber() != my_magic_num_extra))
				continue;
			if (OrderCloseTime() != 0)
				continue;
			int count = 0;
			err = 0;
			RefreshRates();
			int tmp_slippage = cur_slippage;
			writeStr(Day() + "." + Month() + "." + Year() + " " + tmp_str + "log.txt", time_stamp() + "stop buy Bid " + Bid + "\n", 1);
			while ((count<3) && (!OrderClose(tmp_tickets[pos], OrderLots(), Bid, tmp_slippage/*, Green*/)))
			{
				RefreshRates();
				err = GetLastError();
				writeStr(Day() + "." + Month() + "." + Year() + " " + tmp_str + "log.txt", time_stamp() + "stop buy err " + err + "\n", 1);
				if (err == 4108)
				{
					if (OrderSelect(tmp_tickets[pos], SELECT_BY_TICKET, MODE_TRADES) == false)
					{
						err = GetLastError();
						//Comment("error(",err,"): ",ErrorDescription(err));
						//Print("error(",err,"): ",ErrorDescription(err));
						MyAlert("stop buy select ошибка с кодом " + err);
						break;
					}
					if (OrderCloseTime() != 0)
						break;
					else
						MyAlert("stop buy неправильный тикет");
				}
				if (err == 138)
					tmp_slippage += 5;
				if (err == 146)
				{
					while (IsTradeContextBusy())
						Sleep(100);
					continue;
				}
				count++;
			}
			if (count == 3)
			{
				if (err == 138)
					stop_buy_later = true;
				else
					MyAlert("stop buy ошибка с кодом " + err);
			}

			pop_ticket_val(tmp_tickets[pos]);
			buying_count--;
		}
	}
	if (!buying)
	{
		extra_buy_price = 0;
	}
}
void start_sell(bool extra = false)
{
	GetLastError();
	double loc_lot;
	if (!extra)
		if (Risk)
			loc_lot = LOT(Lots, 1);
		else
			loc_lot = Lots;
	else
		if (Risk)
			loc_lot = LOT(ExtraLots, 1);
		else
			loc_lot = ExtraLots;

	int count = 0;
	int err;
	int tmp_slippage = cur_slippage;
	while (count<3)
	{
		RefreshRates();
		color tmp_color = Red;
		if (extra)
			tmp_color = Navy;
		int tmp_magic_num = my_magic_num;
		if (extra)
			tmp_magic_num = my_magic_num_extra;

		double tmp_stoploss = 0;
		if (StopLoss > 0)
			tmp_stoploss = NormalizeDouble(Ask + StopLoss*Point, Digits);
		writeStr(Day() + "." + Month() + "." + Year() + " " + tmp_str + "log.txt", time_stamp() + "start sell lot " + loc_lot + " Ask " + Ask + " Bid " + Bid + "\n", 1);
		sell_ticket = OrderSend(cur_symbol, OP_SELL, loc_lot, MarketInfo(cur_symbol, MODE_BID), tmp_slippage, tmp_stoploss, 0, "up_down sell", tmp_magic_num, tmp_color);
		err = GetLastError();
		if ((sell_ticket == -1) && (err != 1))
		{
			writeStr(Day() + "." + Month() + "." + Year() + " " + tmp_str + "log.txt", time_stamp() + "start sell error " + err + "\n", 1);
			if (err == 138)
				tmp_slippage += 5;
			if (err == 146)
			{
				while (IsTradeContextBusy())
					Sleep(100);
				continue;
			}
			//Comment("error(",err,"): ",ErrorDescription(err));
			//Print("error(",err,"): ",ErrorDescription(err));
		}
		else
		{
			//         Print("sell_ticket = ", sell_ticket);
			selling = true;
			deal_price = Bid;
			stop_price = Ask;
			extra_sell_price = Ask;
			last_min_ask = Ask;
			add_ticket(sell_ticket, tmp_stoploss, Ask, 1, 0);
			start_deal();
			selling_count++;
			break;
		}
		count++;
	}
	if (count == 3)
	{
		if (err == 130)
			MyAlert("start buy ошибка 130 Ask " + Ask + ", Bid " + Bid + ", tmp_tral " + 0 + ", tp_sell " + 0);
		else
			if (err == 138)
				sell_later = true;
			else
				MyAlert("start sell ошибка с кодом " + err);
	}
}

void stop_sell(bool extra = false)
{
	GetLastError();
	if (!extra)
	{
		selling = false;
		stop_deal();
	}
	//OrderClose(sell_ticket,Lots,Ask,3,Red);
	int tmp_size = vect_size;
	for (int pos = 0;pos<tmp_size; pos++)
	{
		tmp_tickets[pos] = tickets[pos];
	}
	for (pos = 0;pos<tmp_size; pos++)
	{
		if (OrderSelect(tmp_tickets[pos], SELECT_BY_TICKET, MODE_TRADES) == false)
			continue;
		int err = GetLastError();
		/*
		if ((err != 0) && (err != 4099))
		{
		//Comment("error(",err,"): ",ErrorDescription(err));
		//Print("error(",err,"): ",ErrorDescription(err));
		MyAlert("stop sell select ошибка с кодом " + err);
		continue;
		}
		*/
		if (OrderType() == OP_SELL)
		{
			if (extra && (OrderMagicNumber() != my_magic_num_extra))
				continue;
			if (OrderCloseTime() != 0)
				continue;
			int count = 0;
			err = 0;
			RefreshRates();
			int tmp_slippage = cur_slippage;
			writeStr(Day() + "." + Month() + "." + Year() + " " + tmp_str + "log.txt", time_stamp() + "stop sell Ask " + Ask + "\n", 1);
			while ((count<3) && (!OrderClose(tmp_tickets[pos], OrderLots(), Ask, tmp_slippage/*, Red*/)))
			{
				RefreshRates();
				err = GetLastError();
				writeStr(Day() + "." + Month() + "." + Year() + " " + tmp_str + "log.txt", time_stamp() + "stop sell err " + err + "\n", 1);
				if (err == 4108)
				{
					if (OrderSelect(tmp_tickets[pos], SELECT_BY_TICKET, MODE_TRADES) == false)
					{
						err = GetLastError();
						//Comment("error(",err,"): ",ErrorDescription(err));
						//Print("error(",err,"): ",ErrorDescription(err));
						MyAlert("stop sell select ошибка с кодом " + err);
						break;
					}
					if (OrderCloseTime() != 0)
						break;
					else
						MyAlert("stop sell неправильный тикет");
				}
				if (err == 138)
					tmp_slippage += 5;
				if (err == 146)
				{
					while (IsTradeContextBusy())
						Sleep(100);
					continue;
				}
				count++;
			}
			if (count == 3)
			{
				if (err == 138)
					stop_sell_later = true;
				else
					MyAlert("stop sell ошибка с кодом " + err);
			}
			pop_ticket_val(tmp_tickets[pos]);
			selling_count--;
		}
	}
	if (!selling)
	{
		extra_sell_price = 0;
	}
}
void stop_trading()
{
	stop_buy();
	stop_sell();
}

//--------------------------------------------------------------------
double LOT(int risk, int ord)
{
	double Lot;
	double MINLOT = MarketInfo(Symbol(), MODE_MINLOT);
	Lot = AccountFreeMargin()*risk / 100 / MarketInfo(Symbol(), MODE_MARGINREQUIRED) / ord;
	if (Lot>MarketInfo(Symbol(), MODE_MAXLOT)) Lot = MarketInfo(Symbol(), MODE_MAXLOT);
	if (StringFind(Symbol(), "XAUUSD") == 0)
		Lot = MathCeil(Lot);
	if (Lot>8) Lot = 8;
	if (Lot<MINLOT) Lot = MINLOT;
	if (MINLOT<0.1) Lot = NormalizeDouble(Lot, 2); else Lot = NormalizeDouble(Lot, 1);

	return(Lot);
}
//--------------------------------------------------------------------
void check_closed()
{
	int tip, Ticket;
	int tmp_size = vect_size;
	int err;
	for (int pos = 0;pos<tmp_size; pos++)
	{
		tmp_tickets[pos] = tickets[pos];
	}
	for (pos = 0;pos<tmp_size; pos++)
	{
		if (OrderSelect(tmp_tickets[pos], SELECT_BY_TICKET, MODE_TRADES) == false)
		{
			/*
			err=GetLastError();
			//Comment("error(",err,"): ",ErrorDescription(err));
			//Print("error(",err,"): ",ErrorDescription(err));
			MyAlert("trailingstop select ошибка с кодом " + err);
			*/
			continue;
		}

		if (OrderCloseTime() != 0)
		{
			pop_ticket_val(tmp_tickets[pos]);

			if (OrderType() == OP_BUY)
			{
				buying_count--;
				if ((buying_count == 0))
				{
					extra_buy_price = 0;
					buying = false;
				}
			}
			if (OrderType() == OP_SELL)
			{
				selling_count--;
				if ((selling_count == 0))
				{
					extra_sell_price = 0;
					selling = false;
				}
			}
			continue;
		}
	}
}
//------------------------------------------------------------------------------
void checkExtra()
{
	double KvarOld;
	if (vect_size >= MaxLots)
		return;
	//Print("buy ", Bid, " ", extra_buy_price*(1+(ExtraOrder/100)));      
	//Print("sell ", Ask, " ", extra_sell_price*(1-(ExtraOrder/100)));
	if (buying && extra_buy_price && (Bid  > (extra_buy_price*(1 + (ExtraOrder / 100)))))
	{
		if ((ExtraTimeframe == 0) || ((KvarExtra > ExtraStochLow) && (KvarExtraOld < ExtraStochLow)))
		{
			comment_msg = comment_msg + "\n" + "EXTRA BUY";
			start_buy(true);
		}
	}
	if (selling && extra_sell_price && (Ask  < (extra_sell_price*(1 - (ExtraOrder / 100)))))
	{
		if ((ExtraTimeframe == 0) || ((KvarExtra < ExtraStochHigh) && (KvarExtraOld > ExtraStochHigh)))
		{
			comment_msg = comment_msg + "\n" + "EXTRA SELL";
			start_sell(true);
		}
	}
}
//------------------------------------------------------------------------------
void checkNoLoss()
{
	for (int pos = 0;pos<vect_size; pos++)
	{
		if (order_types[pos] == 0)
		{
			if ((last_max_bid - open_prices[pos]) < NoLossStart*Point)
				continue;

			int tmp_loss = 0;

			if ((last_max_bid - open_prices[pos]) < NoLossStart2*Point)
				tmp_loss = NoLoss;
			else
				tmp_loss = NoLoss2;

			//Print ( (Bid - open_prices[pos]) , "    ", ( ((100-NoLoss)/100.0) * (last_max_bid - open_prices[pos]) )  );
			if ((Bid - open_prices[pos]) < (((100 - tmp_loss) / 100.0) * (last_max_bid - open_prices[pos])))
			{

				writeStr(Day() + "." + Month() + "." + Year() + " " + tmp_str + "log.txt", time_stamp() + "noloss buy worked\n", 1);
				comment_msg = comment_msg + "\n" + "NOLOSS BUY";
				stop_buy();
				if (selling)
					checkNoLoss();
				return;
			}
		}

		if (order_types[pos] == 1)
		{
			if ((open_prices[pos] - last_min_ask) < NoLossStart*Point)
				continue;

			tmp_loss = 0;

			if ((open_prices[pos] - last_min_ask) < NoLossStart2*Point)
				tmp_loss = NoLoss;
			else
				tmp_loss = NoLoss2;

			//Print ( (open_prices[pos] - Ask), "    ", ( ((100-NoLoss)/100.0) * (open_prices[pos] - last_min_ask) )  );            
			if ((open_prices[pos] - Ask) < (((100 - tmp_loss) / 100.0) * (open_prices[pos] - last_min_ask)))
			{
				writeStr(Day() + "." + Month() + "." + Year() + " " + tmp_str + "log.txt", time_stamp() + "noloss sell worked\n", 1);
				comment_msg = comment_msg + "\n" + "NOLOSS SELL";
				stop_sell();
				if (buying)
					checkNoLoss();
				return;
			}
		}
	}
}
//--------------------------------------------------------------------


void takeProfit()
{
	if (buying)
	{
		if (!buy_profit_armed)
		{
			if (Kvar > TakeProfitHigh)
			{
				comment_msg = comment_msg + "\n" + "TAKEPROFIT BUY ARMED";
				buy_profit_armed = true;
			}
		}
		else
		{
			if (Kvar < TakeProfitHigh)
			{
				comment_msg = comment_msg + "\n" + "TAKEPROFIT BUY WORKED";
				stop_buy();
			}
		}
	}
	if (selling)
	{
		if (!sell_profit_armed)
		{
			if (Kvar < TakeProfitLow)
			{
				comment_msg = comment_msg + "\n" + "TAKEPROFIT SELL ARMED";
				sell_profit_armed = true;
			}
		}
		else
		{
			if (Kvar > TakeProfitLow)
			{
				comment_msg = comment_msg + "\n" + "TAKEPROFIT SELL WORKED";
				stop_sell();
			}
		}
	}
}
//--------------------------------------------------------------------

void check_stopLoss()
{
	double tmp_buy_stop = NormalizeDouble(Bid - StopLoss*Point, Digits);
	double tmp_sell_stop = NormalizeDouble(Ask + StopLoss*Point, Digits);
	for (int pos = 0;pos<vect_size; pos++)
	{
		if (order_types[pos] == 0) //buy
		{
			if (null_passed[pos] == 0)
			{
				if (tmp_buy_stop >(cur_stops[pos] + (StopLoss / 10)*Point))
				{
					OrderSelect(tmp_tickets[pos], SELECT_BY_TICKET, MODE_TRADES);
					Print("old_stop_buy ", cur_stops[pos], " new_stop ", tmp_buy_stop, " Bid ", Bid, " Ask ", Ask);
					comment_msg = comment_msg + "\n" + "old_stop_buy " + cur_stops[pos] + " new_stop " + tmp_buy_stop;
					OrderModify(tmp_tickets[pos], open_prices[pos], tmp_buy_stop, OrderTakeProfit(), 0, CLR_NONE);
					cur_stops[pos] = tmp_buy_stop;
				}

				if (tmp_buy_stop > open_prices[pos])
				{
					null_passed[pos] = 1;
					comment_msg = comment_msg + "\n" + "BUY NULL PASSED";
				}
			}

		}

		if (order_types[pos] == 1) //sell
		{
			if (null_passed[pos] == 0)
			{
				Print("tmp_sell_stop ", tmp_sell_stop, " val ", (cur_stops[pos] - (StopLoss / 10)*Point));
				if (tmp_sell_stop < (cur_stops[pos] - (StopLoss / 10)*Point))
				{
					OrderSelect(tmp_tickets[pos], SELECT_BY_TICKET, MODE_TRADES);
					Print("old_stop_sell ", cur_stops[pos], " new_stop ", tmp_sell_stop, " Bid ", Bid, " Ask ", Ask);
					comment_msg = comment_msg + "\n" + "old_stop_sell " + cur_stops[pos] + " new_stop " + tmp_sell_stop;
					OrderModify(tmp_tickets[pos], open_prices[pos], tmp_sell_stop, OrderTakeProfit(), 0, CLR_NONE);
					cur_stops[pos] = tmp_sell_stop;
				}
				if (tmp_sell_stop < open_prices[pos])
				{
					null_passed[pos] = 1;
					comment_msg = comment_msg + "\n" + "SELL NULL PASSED";
				}
			}
		}

	}
}

//--------------------------------------------------------------------

int init()
{

	if (!IsTradeAllowed())
		MyAlert(Symbol() + " —ќ¬≈“Ќ» ” «јѕ–≈ў≈Ќќ “ќ–√ќ¬ј“№!!!!");

	if (!IsDllsAllowed())
		MyAlert(Symbol() + " DLL Ќ≈ –ј«–≈Ў≈Ќџ!!!!");
	tmp_str = AccountServer() + " " + Symbol() + Period();
	Print("»нформаци€ дл€ справки. ¬рем€ сервера " + Hour() + ":" + Minute());
	cur_symbol = Symbol();
	my_magic_num = Period();
	my_magic_num_extra = my_magic_num + 100000;
	vect_size = 0;
	cur_slippage = 5;
	if (StringFind(cur_symbol, "EURUSD") == 0)
		cur_slippage = 3;
	if (StringFind(cur_symbol, "GBPUSD") == 0)
		cur_slippage = 5;
	if (StringFind(cur_symbol, "USDCHF") == 0)
		cur_slippage = 5;
	if (StringFind(cur_symbol, "USDJPY") == 0)
		cur_slippage = 7;
	bar_size = Point;
	double tmp_ = (Ask - Bid) / Point;
	Print("Ask - Bid in points ", tmp_);
	double OpenClose = 0;
	double HighLow = 0;
	tick_time = 0;
	for (int i = 1; i <= 500; i++)
	{
		OpenClose += MathAbs(Close[i] - Open[i]);
		HighLow += MathAbs(High[i] - Low[i]);
		tick_time += Volume[i];
	}
	Print("Close - Open average in points ", OpenClose / 1000 / Point);
	Print("High - Low average in points ", HighLow / 1000 / Point);
	stat_bar_size = HighLow / 1000 / Point;
	tick_time /= 500;
	Print("Average ticks in bar ", tick_time);
	double min_stop = MarketInfo(Symbol(), MODE_STOPLEVEL);
	Print("Minimum stop ", min_stop*Point, " (", min_stop, " points)");
	if (StopLoss < min_stop)

		tick_time = (Time[1] - Time[2]) / TickCount;
	prev_time = TimeCurrent();
	//stop_trading();
	last_price = cur_price();
	buying_count = 0;
	selling_count = 0;
	up_passed = false;
	down_passed = false;
	last_max_rsi = 0;
	//ObjectCreate("dasd", OBJ_HLINE, 0, Time[0], Ask);
	bar_time = Time[0];

	last_min_ask = Ask;
	last_max_bid = Bid;

	for (int ii = 0; ii<OrdersTotal(); ii++)
	{
		if (OrderSelect(ii, SELECT_BY_POS, MODE_TRADES) == true)
		{
			if ((OrderSymbol() == Symbol()) && ((OrderMagicNumber() == my_magic_num) || (OrderMagicNumber() == my_magic_num_extra)))
			{
				int _type = 0;
				int _null_passed = 0;
				if (OrderType() == OP_BUY)
				{
					buying = true;
					buying_count++;
					_type = 0;
					if (OrderStopLoss() > (OrderOpenPrice() + MarketInfo(cur_symbol, MODE_BID)))
						_null_passed = 1;
				}
				else
				{
					selling = true;
					if (OrderStopLoss() < (OrderOpenPrice() - MarketInfo(cur_symbol, MODE_ASK)))
						_null_passed = 1;
					selling_count++;
					_type = 1;
				}

				add_ticket(OrderTicket(), OrderStopLoss(), OrderOpenPrice(), _type, _null_passed);
			}
		}
	}
}


int start()
{
	//         ticket=OrderSend(Symbol(),OP_BUY,Lots,Ask,3,0,Ask+TakeProfit*Point,"macd sample",16384,0,Green);
	//        if(OrderSelect(ticket,SELECT_BY_TICKET,MODE_TRADES)) Print("BUY order opened : ",OrderOpenPrice());
	//         ticket=OrderSend(Symbol(),OP_SELL,Lots,Bid,3,0,Bid-TakeProfit*Point,"macd sample",16384,0,Red);
	//         if(OrderSelect(ticket,SELECT_BY_TICKET,MODE_TRADES)) Print("SELL order opened : ",OrderOpenPrice());
	//        Print("wrong ", DoubleToStr(cur_price(), Digits), " ", DoubleToStr(last_price, Digits));
	// не работает в выходные дни.
	comment_msg = " ";
	if (bar_time != Time[0])
	{
		bar_time = Time[0];
		new_bar = true;
	}
	if (!IsTesting())
	{
		read_command();
		tell_my_name();
	}
	if (manual_mode)
	{
		if (buying || selling)
			check_closed();

		if (buying || selling)
		{
			checkNoLoss();

			if (Ask < last_min_ask)
				last_min_ask = Ask;


			if (Bid > last_max_bid)
				last_max_bid = Bid;
		}
		return (0);
	}

	cur_ticks++;
	if (cur_ticks < TickCount)
		return(0);

	cur_ticks = 0;


	int tmp_hour = Hour() + 2;
	if (tmp_hour > 23)
		tmp_hour -= 24;
	if (CloseFriday != 0)
	{
		if (DayOfWeek() == 0 || DayOfWeek() == 6) comment_msg = comment_msg + "\n" + "WEEK_DAY ERROR";

		/*
		if (freeze_trading && (DayOfWeek() == 1))
		freeze_trading = false;
		*/

		if ((DayOfWeek() == 5) && (tmp_hour == 23) && (Minute() > 55))
		{
			if (dealing)
			{
				if (CloseFriday == 1)
				{
					stop_buy();
					stop_sell();
				}
				else
				{
					stop_buy(true);
					stop_sell(true);
				}
			}
			return (0);
		}

	}
	if (freeze_trading)
		return(0);
	/*
	int BeginHour = MathFloor(BeginTime);
	int BeginMinute = (BeginTime - BeginHour)*100;
	int EndHour = MathFloor(EndTime);
	int EndMinute = (EndTime - EndHour)*100;
	*/


	if (BeginTime != -1)
	{
		if ((tmp_hour * 100 + Minute()) < (BeginTime * 100))
			return (0);

		if ((tmp_hour * 100 + Minute()) > (EndTime * 100))
		{
			if (dealing)
			{
				stop_buy();
				stop_sell();
			}
			return (0);
		}
	}
	/*
	if (new_bar)
	{
	*/
	PlaySound("tick.wav");
	RefreshRates();
	int retries = 0;
	int max_retries = 4;
	int err = 0;
	GetLastError();
	for (retries = 0; retries < max_retries; retries++)
	{
		Kvar = iStochastic(NULL, MainTimeFrame, KPeriod, DPeriod, Slowing, MAMethod, PriceField, MODE_MAIN, 0);
		err = GetLastError();
		if (err == 0)
			break;
		if (err != 4066)
		{
			MyAlert("Kvar main ошибка с кодом " + err);
			continue;
		}
		Sleep(3000);
		RefreshRates();
	}
	if (retries == max_retries)
		MyAlert("Kvar main 4066 не решилась");
	if (ExtraTimeframe != 0)
	{
		for (retries = 0; retries < max_retries; retries++)
		{
			KvarExtra = iStochastic(NULL, ExtraTimeframe, KPeriod, DPeriod, Slowing, MAMethod, PriceField, MODE_MAIN, 0);
			err = GetLastError();
			if (err == 0)
				break;
			if (err != 4066)
			{
				MyAlert("Kvar extra ошибка с кодом " + err);
				continue;
			}
			Sleep(3000);
			RefreshRates();
		}
		if (retries == max_retries)
			MyAlert("Kvar extra 4066 не решилась");
	}

	for (retries = 0; retries < max_retries; retries++)
	{
		Dvar = iStochastic(NULL, MainTimeFrame, KPeriod, DPeriod, Slowing, MAMethod, PriceField, MODE_SIGNAL, 0);
		err = GetLastError();
		if (err == 0)
			break;
		if (err != 4066)
		{
			MyAlert("Dvar main ошибка с кодом " + err);
			continue;
		}
		Sleep(3000);
		RefreshRates();
	}
	if (retries == max_retries)
		MyAlert("Dvar main 4066 не решилась");

	comment_msg = comment_msg + "K " + Kvar + " D " + Dvar;

	if (buying || selling)
	{
		if (Dvar > (Kvar + Kdop))
		{
			if (buying)
			{
				comment_msg = comment_msg + "\n" + "SELL CROSS";
				stop_buy();
			}
			if (!selling)
				start_sell();
			//new_bar = false;
		}
		if (Kvar > (Dvar + Ddop))
		{
			if (selling)
			{
				comment_msg = comment_msg + "\n" + "BUY CROSS";
				stop_sell();
			}
			if (!buying)
				start_buy();
			//new_bar = false;
		}
	}
	else
	{
		if ((KvarOld > (DvarOld + Ddop)) && (Dvar > (Kvar + Kdop)))
		{
			if (!selling)
			{
				comment_msg = comment_msg + "\n" + "SELL START CROSS";
				start_sell();
			}
			//new_bar = false;
		}
		if ((DvarOld > (KvarOld + Kdop)) && (Kvar > (Dvar + Ddop)))

		{
			if (!buying)
			{
				comment_msg = comment_msg + "\n" + "BUY START CROSS";
				start_buy();
			}
			//new_bar = false;
		}
	}
	if ((ExtraTimeframe != 0) && (buying || selling))
	{
		double DvarExtra;

		for (retries = 0; retries < max_retries; retries++)
		{
			DvarExtra = iStochastic(NULL, ExtraTimeframe, KPeriod, DPeriod, Slowing, MAMethod, PriceField, MODE_SIGNAL, 0);
			err = GetLastError();
			if (err == 0)
				break;
			if (err != 4066)
			{
				MyAlert("Dvar extra ошибка с кодом " + err);
				continue;
			}
			Sleep(3000);
			RefreshRates();
		}
		if (retries == max_retries)
			MyAlert("Dvar extra 4066 не решилась");
		if (DvarExtra > (KvarExtra + Kdop))
		{
			if (buying)
				stop_buy(true);
			//new_bar = false;
		}
		if (KvarExtra > (DvarExtra + Ddop))
		{
			if (selling)
				stop_sell(true);
			//new_bar = false;
		}
	}
	//}
	///////////////////////////////////////////////////////////////////////   
	///////////////////////////////////////////////////////////////////////   
	///////////////////////////////////////////////////////////////////////   
	if (buy_later)
	{
		Print("ќтложенный buy. Ё“ќ Ќќ–ћјЋ№Ќќ просто дл€ статистики");
		start_buy();
		buy_later = false;
	}

	if (sell_later)
	{
		Print("ќтложенный sell. Ё“ќ Ќќ–ћјЋ№Ќќ просто дл€ статистики");
		start_sell();
		sell_later = false;
	}

	if (stop_buy_later)
	{
		Print("ќтложенный stop buy. Ё“ќ Ќќ–ћјЋ№Ќќ просто дл€ статистики");
		stop_buy();
		stop_buy_later = false;
	}

	if (stop_sell_later)
	{
		Print("ќтложенный stop sell. Ё“ќ Ќќ–ћјЋ№Ќќ просто дл€ статистики");
		stop_sell();
		stop_sell_later = false;
	}


	if (buying || selling)
		check_closed();

	if (buying || selling)
		checkExtra();

	KvarExtraOld = KvarExtra;
	KvarOld = Kvar;
	DvarOld = Dvar;

	if (buying || selling)
	{
		checkNoLoss();

		if (Ask < last_min_ask)
			last_min_ask = Ask;


		if (Bid > last_max_bid)
			last_max_bid = Bid;

		if (StopLoss != 0)
			check_stopLoss();
	}

	if (!buying)
		buy_profit_armed = false;

	if (!selling)
		sell_profit_armed = false;

	if (buying || selling)
		takeProfit();

	if (StringFind(comment_msg, "\n", 0) != -1)
	{
		PlaySound("alert2.wav");
		writeStr(Day() + "." + Month() + "." + Year() + " " + tmp_str + "log.txt", time_stamp() + comment_msg, 1);
	}
	else
	{
		//writeStr(Day()+"."+Month()+"."+Year()+" "+tmp_str+"log.txt", time_stamp()+comment_msg+"\n", 1);   
	}

	Comment(comment_msg + "\n------------------------------\n" + old_comment_msg_1 + "\n------------------------------\n" + old_comment_msg_2 + "\n------------------------------\n" + old_comment_msg_3);
	old_comment_msg_3 = old_comment_msg_2;
	old_comment_msg_2 = old_comment_msg_1;
	old_comment_msg_1 = comment_msg;

	return(0);
}
// the end.
from Compare import CompareStrategy
from visualization import StrategyAnalysis

fill_path = "sample_data/BACK_TKSPY_2022-04-02_143804_start_10-30-2019_end_10-30-2019_fill.csv"
order_path = "sample_data/BACK_TKSPY_2022-04-02_143804_start_10-30-2019_end_10-30-2019_order.csv"
pnl_path = "sample_data/BACK_TKSPY_2022-04-02_143804_start_10-30-2019_end_10-30-2019_pnl.csv"
my_strategy1 = StrategyAnalysis(fill_file=fill_path, order_file=order_path, pnl_file=pnl_path, initial_value=10000000)

fill_path = "sample_data/BACK_NONSPY_2022-04-02_143804_start_10-30-2019_end_10-30-2019_fill.csv"
order_path = "sample_data/BACK_NONSPY_2022-04-02_143804_start_10-30-2019_end_10-30-2019_order.csv"
pnl_path = "sample_data/BACK_NONSPY_2022-04-02_143804_start_10-30-2019_end_10-30-2019_pnl.csv"
my_strategy2 = StrategyAnalysis(fill_file=fill_path, order_file=order_path, pnl_file=pnl_path, initial_value=10000000)



my_strategy1.visualizePNL()
my_strategy2.visualizePNL()

cs = CompareStrategy()
cs.addStrategy(my_strategy1)
cs.addStrategy(my_strategy2)

cs.visualizeStrategies()
cs.measurementTable()
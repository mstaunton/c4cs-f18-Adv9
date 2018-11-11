import gdb

class OrderPrinter:
	def __init__(self,val):
		self.val = val

	def to_string(self):
		if self.val['type'] == 'b':
			order_type = 'BUY'
		elif self.val['type'] == 's':
			order_type = 'SELL'
		else:
			order_type = 'WRONG'

		stock_num = self.val['stock_num']
		price = self.val['price']

		return'Order Type: {}, Stock Number: {}, Price: {}\n'.format(order_type,stock_num,price)

	def lookup_type(val):
		if str(val.type) == 'Order':
			return OrderPrinter(val)

	gdb.prettyprinters.append(lookup_type)

# General factorial function written in Python.
# Works transparently with integers and floating point.
# Gives an error for negative or non-integer input values.

def fact(x):
	"Return x! (x factorial)."
	if (x < 0 or (x % 1.0) != 0.0):
		raise ValueError("Factorial argument must be a positive integer.")
		return
	if (x == 0):
		return x + 1
	d = x
	while (x > 2):
		x -= 1
		temp = d * x
		if (temp <= d):
			raise ValueError("Factorial result too large.")
			return
		d = temp
	return d

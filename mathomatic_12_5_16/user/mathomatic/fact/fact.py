# General factorial function written in Python.
# Works transparently with integers and floating point.
# Gives an error for negative or non-integer input values.

def fact(x):
	"Return x! (x factorial)."
	if (x < 0 or (x % 1.0) != 0.0):
		raise FloatingPointError, "Factorial argument must be a positive integer."
		return
	if (x == 0):
		return x + 1
	d = x
	while (x > 1):
		x -= 1
		d *= x
	return d

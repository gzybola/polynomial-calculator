# polynomial-calculator
Multivariate polynomial RPN calculator written for Invidual Programming Project course at University of Warsaw 
## Polynomial format
Polynomials is either an integer constant, a monomian or sum of monomians. A monomian is represented as (coeff, exp), where coeff is a polynomial and exp is unsigned integer exponent.</br>
#### Examples of good polynomial:

* (1,2)+(1,0)
* (1,2)+(-1,2)
* (1,2)+(-2,2)
* ((1,2),15)+(-7,8)
* (3,1)+(((4,4),100),2)



## Command list


* `ZERO` – pushes zero polynomial to stack
* `IS_COEFF` – checks whether top polynomial is a constant coefficient
* `IS_ZERO` - checks whether top polynomial is a zero polynomial
* `CLONE` = pushes a copy of top polynomial to stack
* `ADD` - pops two top polynomials and pushes their sum to stack
* `MUL` - pops two top polynomials and pushes their product to stack
* `NEG` - pops top polynomial and pushes its negation to stack
* `SUB` - pops two top polynomials and pushes their difference to stack
* `IS_EQ` - checks whether two top polynomials are equal
* `DEG` - prinst a degree of top polynomial
* `DEG_BY` - prints a degree relative to variable x_i of top polynomial
* `AT` x - pops top polynomial, calculates its value in x and pushes it to stack
* `PRINT` - prinst top polynomial in the simplest format
* `POP` - pops top polynomial

## Test script
Runs with two arguments: name of program and directory to tests.

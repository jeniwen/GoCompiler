//Cannot index arrays with statements (last line)
package main

var x [3]int

func main ()  {
  x[0] = 1
  x[1] = 2
  x[2] = 3
  x[1+2+3+4+5+6] = 7
  x[x] = 5;
  x[x++] = 5
}

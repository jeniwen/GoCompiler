//Redeclaration of type
package main

func main () {
  type (
    num int
    point struct {
      x, y float64
    }
  )
  type num1 num;
  type num1 bool;
}

//Accessing type in sibling scope
package main

func main () {
  type (
    num int
    point struct {
      x, y float64
    }
  )
  type num1 num;
}

func pain (){
  type num2 num1
  return;
}

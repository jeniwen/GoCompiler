//Bad else if condition
package main
func main () {
  var x int = 5;
  if x:=8+9; x < 5 {
    print(x+1);
  } else if x >= 6 {
    print(x+2);
  } else if x << 8 { //x << 8 doesn't result in bool
    print(x+3);
  }
}

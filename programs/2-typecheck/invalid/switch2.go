//Non-comparable switch expression
package main
func main () {
  type student struct {
    id int;
    gpa float64;
  }
  type prof struct {
    id int;
    tenure bool;
  }
  var p prof;
  var s student;

  switch p == s {
  case true: print("nope")
  case false: print("doesn't typecheck :()")
  default:
  }

}

//Non-bool cases of a switch statement without an expression
package main
func main () {
  switch {
  case 1,2: print("nope")
  case 3,4: print("doesn't typecheck :()")
  default:
  }

}

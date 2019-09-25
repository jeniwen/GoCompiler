//Case expr does not have same type as switch expression
package main
func main () {
  switch x:="string"; x < "strong" {
  case true: print(x)
  case false: print(x)
  case 2: print(" ")
  default:
  }

}

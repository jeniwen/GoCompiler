//bad var declaration
package main
func main() {
  var i = 5
  var (
    x1, x2 int
    y1, y2 = 42, 43
    z1, z2 int = 1, 2
    z1 int = 1, 2
  )

  var j int = 6
  var int int = 7;

  var ()
  var (a) = 5

}

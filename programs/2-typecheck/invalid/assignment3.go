//Multiple assignment, last one of wrong type
package main

func main () {
  var x[3] int
  type student struct {
    id int
    gpa float64
  }
  var naruto student
  var y = 3
  x[0], naruto.gpa, y = 2, 1.30, `false`; //assigning string to y
}

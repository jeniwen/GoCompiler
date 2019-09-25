//Printing a struct
package main

func main () {
  type student struct {
    id int
    gpa float64
  }
  var sasuke student
  print(1, 0x00, 07, true,'\n',`raw`, "string");
  println(1,true,'\n',`raw`, "string");
  print(sasuke);
}

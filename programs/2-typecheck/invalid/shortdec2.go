//All variables on LHS of declaration has already been declared
package main

func pain () { //should be legal
  var x, y bool
  x, y, z := true, `false`, true != false;
}

func main () { //Illegal
  var x, y, z bool
  x, y, z := true, `false`, true == false;
}

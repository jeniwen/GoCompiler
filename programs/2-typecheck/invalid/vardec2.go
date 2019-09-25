//Var reference in an sibling scope

package main

func main () {
 var x = 5;
 var y = x;
 x = y;
}

func pain () {
  var y = x;
  return;
}

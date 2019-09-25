//Var reference in an parent scope

package main

func main () {
 var x = 5;
 var y = x;
 x = y;
}

var z int = x;

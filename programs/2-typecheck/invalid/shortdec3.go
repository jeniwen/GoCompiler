//Already declared variables of short dec not assigned same type
package main

func pain () { //should be legal
  var x, y bool
  x, y, z := true, `false`, true != false;
}

func rain () { //should be illegal
  var x, y bool
  x, y, z := true, `false`, 4;
}

func rain () { //Illegal

}

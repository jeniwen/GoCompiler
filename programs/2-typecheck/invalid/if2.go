//Bad if statement initialization
package main
func main () {
  var x int = 5;
  if x:=true + `false`; x == false {
    print(x);
  }

}

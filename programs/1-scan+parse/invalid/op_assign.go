// Can't use list with op-assign

package main

func main(){
	var a, b int = 5,6
	a, b += 9 , 7
}

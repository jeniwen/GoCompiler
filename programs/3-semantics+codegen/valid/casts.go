//~A a 
//~ #deep
//~3 3 98 10
//~+6.400000e+001 +3.140000e+000 +9.700000e+001 +1.000000e+001
//~A
//~A a 
//~ #deep
//~3 3 98 10
//~+6.400000e+001 +3.140000e+000 +9.700000e+001 +1.000000e+001


package main

func foo(s string) {
        println(s);
}

func main() {
        var x rune = '\n';
        println(string(65), string('a'), string('\n'), string("#deep"));
        println(int(3.14), int(3), int('b'), int('\n'));
        println(float64(64), float64(3.14), float64('a'), float64('\n'));
        foo(string(65));

        type num int
        type flt float64
        type run rune
        type str string

        println(str(65), str('a'), str('\n'), str("#deep"));
        println(num(3.14), num(3), num('b'), num('\n'));
        println(flt(64), flt(3.14), flt('a'), flt('\n'));

}

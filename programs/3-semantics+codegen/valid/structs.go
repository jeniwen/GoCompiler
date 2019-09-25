//~0 1 2
//~3 0 0
//~1
//~0
//~+3.140000e+000 +2.710000e+000
//Struct testing, with arrays and slices >:) i like writing test programs too much does that make me mean
//NOT BENCHMARK
package main

type a struct {
        a int
        int int
        b int
}

type struct_o_struct struct{
        field2 struct {
                a2 int;
                int2 int;
                b2 int;
        }
}


type struct_o_struct_o_struct struct{
        field3 struct {
                a3 int;
                int3 int;
                field3 struct_o_struct;
        }
}

type struct_o_struct_o_struct2 struct{
        field struct {
                a3 int;
                int3 int;
                field struct_o_struct_o_struct;
        }
}

type struct_o_array struct{
        arr1 [10]float64;
        slice1 []float64;
}

func main () {
        var struct1 a;
        struct1.a = 0;
        struct1.int = 1;
        struct1.b = 2;
        println(struct1.a, struct1.int , struct1.b)

        var s2 struct_o_struct;
        s2.field2.a2 = 3;
        println(s2.field2.a2, s2.field2.int2 , s2.field2.b2)

        var s5 struct_o_struct_o_struct;
        s5.field3.field3.field2.a2 = 1;
        println(s5.field3.field3.field2.a2);

        var s7 struct_o_struct_o_struct2;
        println(s7.field.field.field3.a3);


        var s3 struct_o_array;
        s3.arr1[0] = 3.14;
        s3.slice1 = append(s3.slice1, 2.71)
        println(s3.arr1[0], s3.slice1[0])

}

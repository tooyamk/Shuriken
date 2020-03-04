shader {
    define {
        static {
            aabb     1.0
        }

        dynamic {

        }
    }

    program {
        vs {
            #include "modelVert.hlsl"
        }

        ps {
            #include "modelFrag.hlsl"
        }
    }

    /*
    variant {
        define {

        }

        program {
            vs {

            }

            ps {

            }
        }
    }
    */
}
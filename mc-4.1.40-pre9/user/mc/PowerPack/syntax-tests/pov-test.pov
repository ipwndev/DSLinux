// a mountain cabin
// by Kurt Bangert (bangert@sirius.lahn.de)

#declare EMBWood1 = 
texture {  /* Bottom wood-grain layer */
    pigment {
        wood
        turbulence 0.05
        color_map {
            [0.00 rgb <0.58, 0.45, 0.23>]
            [0.34 rgb <0.65, 0.45, 0.25>]
            [0.40 rgb <0.33, 0.23, 0.13>]
            [0.47 rgb <0.60, 0.40, 0.20>]
            [1.00 rgb <0.25, 0.15, 0.05>]
        }
    }
    finish {
        crand 0.02
        ambient 0.32
        diffuse 0.63
        phong 0.2
        phong_size 10
    }
    normal { bumps 0.05 }
}
texture {     /* top layer, adds small dark spots */
    pigment {
        bozo
        color_map {
            [0.0 rgbf <1.00, 1.00, 1.00, 1.00>]
            [0.8 rgbf <1.00, 0.90, 0.80, 0.80>]
            [1.0 rgbf <0.30, 0.20, 0.10, 0.40>]
        }
    scale 0.25
    }
}

#declare r1 = seed(1); 

//snow on the ground                       
#declare snow_patch = object { blob {
 threshold 0.5
  component 1,.6,  <-2.5, .5,  0>   component 1,1    <2.5,  0,  -1>
  component 1,1,   <-2,   .4,  0>   component 1,1.3, <2,    0,  -1>
  component 1,1.2, <-1.5,  0,  0>   component 1,.8,  <1.2,  .3, -1>
  component 1,1.1, <0,     0,  0>   component 1,1.1, <0,    0,  -1>
  component 1,.8,  <1.2,  .3,  0>   component 1,1.2, <-1.5, 0,  -1>
  component 1,1.3, <2,     0,  0>   component 1,1,   <-2,   .4, -1>
  component 1,1    <2.5,   0,-.5>   component 1,2,   <-2.5, .5, -1>
  component 1,.8,  <1.,   .3, -1>   component 1,.8,  <1.2,  .3, .5>

 texture { pigment { White } finish { ambient .2 diffuse .3 }}}
}

#declare i=0;
#declare wall=union{
#while (i<13)
 object {cylinder {<-10,i*.3,0> <10,i*.3,0> .15 texture {EMBWood1 translate <rand(r1*5),rand(r1*5),rand(r1*5)> scale y*rand(r1)*.2 }}}
#declare i=i+1;
#end                
}                     

#declare walls=
difference{
union{
object {wall}
object {wall translate <0,0,9.30>}
object {wall scale x*.5 rotate y*90 translate <9.65,0,4.65>}
object {wall scale x*.5 rotate y*90 translate <-9.65,0,4.65>}
     }          
object {box{<-1,1.3,-1><1,3.3,11>}}
object {box{<-1,1.3,-1><1,3.3,11>}translate -x*4}
object {box{<-1,1.3,-1><1,3.3,11>}translate x*4}
object {box{<-1,1.3,-1><1,3.3,11>}translate -x*7}
object {box{<-1,1.3,-1><1,3.3,11>}translate x*7}
}

#declare cross=union{
object{box{<-1,2.18,-.1><1,2.42,.1> }}
object{box{<-.12,1.3,-.1><.12,3.3,.1> }}
}

#declare chimney=union{
cylinder{<0,0,0><0,7,0>.1 }
cylinder{<-.3,6.9,0>< .3,6.9,0> .1 }
cylinder{<-.3,6.5,0><-.3,7.7,0> .1 }
cylinder{< .3,6.5,0>< .3,7.7,0> .1 }
}

#declare cabin= union{
object{walls}  
object {box{<-1,1.3,0><1,3.3,0.1>}pigment {color rgbt<.3,.3,0,.8>}finish {Luminous}}
object {cross texture {EMBWood1 translate x*rand(r1)}}
object {cross translate -x*4  texture {EMBWood1 translate x*rand(r1)}}
object {box{<-1,1.3,0><1,3.3,0.1>}translate x*4 pigment {color rgbt<.3,.3,0,.5>}finish {Luminous}}
object {cross translate x*4 texture {EMBWood1 translate x*rand(r1)}}
object {box{<-1,1.3,0><1,3.3,0.1>}translate -x*7 pigment {color rgbt<.5,.5,0,.5>}finish {Luminous}}
object {cross translate -x*7 texture {EMBWood1 translate x*rand(r1)}}
object {box{<-1,1.3,0><1,3.3,0.1>}translate x*7 pigment {color rgbt<.3,.3,0,0.5>}finish {Luminous}}
object {cross translate x*7 texture {EMBWood1 translate x*rand(r1)}}
//a visible light in the window
sphere {<-7.6,1.5,.2> .2 pigment {color rgb<1.5,1,0>}finish {Luminous} }
light_source { <-7.6, 1.5, -.1> color rgb <1.5,1,0> fade_distance .1 fade_power 2 }    
object{box{<3,0,0><3,4,5> pigment {Gray50}}}
object{box{<5,0,0><5,4,5> pigment {Gray50}}}
object{box{<3,0,5><5,4,5.1> pigment {Gray50}}}

//the roof is not needed at the moment
//object{box{<-11,0,-1><11,.5,5> rotate -x*10 translate <0,4,0> texture { pigment { Gray50 } normal {bumps 0.5} finish { ambient .7 diffuse .3 }}}}
//object{box{<-11,0,-1><11,.5,5> rotate x*10 translate <0,4.7,5.7> texture { pigment { White } normal {bumps 0.5} finish { ambient .7 diffuse .3 }}}}

//chimney
object {chimney translate <-5,0,5> pigment {Gray50}}
object {chimney translate <5,0,5> pigment {Gray50}}

//patches of snow on the roof
object { snow_patch scale <3.5,.5,3> rotate -y*3 rotate -x*10 translate <1,5.7,7.5>  }
object { snow_patch scale <3.5,1,3> rotate -y*3 rotate -x*10 translate <1,5,5>  }
object { snow_patch scale <2,.7,3> rotate -y*5 rotate -x*10 translate <2,4.9,3.3>  }
//object { snow_patch scale <3.3,.7,1> rotate -y*5 rotate -x*10 translate <0,4.3,-.1>  }
}

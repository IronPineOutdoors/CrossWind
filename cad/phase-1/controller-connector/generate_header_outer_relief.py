"""Relieve only the outboard short edge of the four-pin shroud aperture."""
import manifold3d as m
from generate_header_fit_tests import body_coupon, box, export

if __name__ == '__main__':
    parts=[]
    for n,extra in enumerate((.2,.4),1):
        part=body_coupon(0.0,n)  # Prior refinement 3-dot dimensions.
        cutter=box((13+extra,7,4),(26,4,-1))
        part-=cutter
        assert (part ^ cutter).volume()<1e-7
        # Two bars distinguish outer-relief tests from prior single-bar series.
        for y in (13.2,15.6):
            part+=box((6,1.2,.6),(34,y,2))
        export(part,f'PCB_Header_Outer_Relief_{n}dot_PRINT.stl',1)
        parts.append(part.translate((0,(n-1)*23,0)))
    export(m.Manifold.batch_boolean(parts,m.OpType.Add),
           'PCB_Header_Outer_Relief_All_PRINT.stl',2)

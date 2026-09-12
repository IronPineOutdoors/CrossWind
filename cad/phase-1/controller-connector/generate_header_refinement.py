"""Smaller PCB-header body coupons following first physical fit feedback."""
import manifold3d as m
from generate_header_fit_tests import body_coupon, box, export

if __name__ == '__main__':
    parts=[]
    # Ascending sizes: all are smaller than the original 1-dot coupon.
    for n,offset in enumerate((-.4,-.2,0.0),1):
        part=body_coupon(offset,n)
        # Raised bar distinguishes this refinement series from original coupons.
        part += box((6,1.2,.6),(34,14.4,2))
        export(part,f'PCB_Header_Refine_{n}dot_PRINT.stl',1)
        parts.append(part.translate((0,(n-1)*23,0)))
    export(m.Manifold.batch_boolean(parts,m.OpType.Add),
           'PCB_Header_Refine_All_PRINT.stl',3)

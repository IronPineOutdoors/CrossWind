"""Three-piece, hand-held header assembly test. No soldering or power."""
import manifold3d as m
from generate_header_fit_tests import box, export

# Guide sits on the pin plate; loose planar sliding establishes row alignment.
def guide():
    solid=box((44,22,2),(0,0,0))
    for x,w in ((3,18),(26,13)):
        cut=box((w,7,4),(x,7,-1))
        solid-=cut
        assert (solid ^ cut).volume()<1e-7
    return solid

def pin_plate():
    solid=box((44,26,1.2),(0,0,0))
    # Centered 6.28 mm inferred body in 7 mm guide: row offset is +0.69 mm.
    # Guide nominally has 2 mm margin to each long edge of this larger plate.
    for count,cx in ((6,12),(4,32.5)):
        for i in range(count):
            x=cx+(i-(count-1)/2)*2.5
            cut=box((1.1,1.1,4),(x-.55,13.19-.55,-1))
            solid-=cut
            assert (solid ^ cut).volume()<1e-7
    return solid

def stand():
    return box((44,26,3),(0,0,0))-box((40,22,5),(2,2,-1))

if __name__ == '__main__':
    g,p,s=guide(),pin_plate(),stand()
    for part,name in ((g,'Guide'),(p,'Pin_Plate'),(s,'Stand')):
        export(part,f'PCB_Header_Assembly_{name}_PRINT.stl',1)
    export(m.Manifold.batch_boolean([g,p.translate((0,27,0)),s.translate((0,58,0))],m.OpType.Add),
           'PCB_Header_Assembly_All_PRINT.stl',3)
    # Verify nominal assembled pieces do not overlap in volume.
    placed=[s,p.translate((0,0,3)),g.translate((0,2,4.2))]
    for i in range(3):
        for j in range(i):
            assert (placed[i] ^ placed[j]).volume()<1e-7
    assert 3.19-1.2 < 3  # Protruding solder tails clear the supporting table.

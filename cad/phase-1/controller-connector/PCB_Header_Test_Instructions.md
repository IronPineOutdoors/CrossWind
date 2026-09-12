# PCB-header fit tests - 2026-09-12

Print PCB_Header_All_Tests_PRINT.stl for all four pieces (44 x 101 mm footprint).
Individual PCB_Header_Body_1dot_PRINT.stl, PCB_Header_Body_2dot_PRINT.stl,
PCB_Header_Body_3dot_PRINT.stl and PCB_Header_Tails_PRINT.stl are also supplied.
These are new PCB-header tests, distinct from the older cable-housing gauges.

Print flat as exported in PETG, 0.4 mm nozzle, 0.2 mm layers, four walls,
solid fill, no supports. Body plates are 2 mm thick; tail plate is 1.2 mm thick.
Raised dots add 0.6 mm. Remove strings and first-layer burrs before testing.

## Body coupons

Each coupon has a six-pin opening on the left and a four-pin opening on the
right when the raised dots are behind the openings. Insert each UNMATED PCB
header's solder tails first, then check whether its plastic shroud enters the
2 mm plate without force. A body shoulder may stop it; do not force that through.
These rectangular openings check shroud clearance, not retention or hook fit.

The photo-estimated starting envelopes are 18 x 7 mm for six-pin and 13 x 7 mm
for four-pin headers. These are provisional, not caliper measurements.

| Dots | Six-pin opening | Four-pin opening |
| --- | --- | --- |
| 1 | 18.2 x 7.2 mm | 13.2 x 7.2 mm |
| 2 | 18.6 x 7.6 mm | 13.6 x 7.6 mm |
| 3 | 19.0 x 8.0 mm | 14.0 x 8.0 mm |

Start with three dots and work down. Report the smallest comfortable fit for each
header separately. If none fits, report whether the long or short dimension
catches. Do not force or file the connector bodies.

## Tail coupon

The larger 44 x 32 mm plate has two rows, each with six holes on the left and
four on the right. One dot identifies 0.9 mm square holes; two dots identifies
1.1 mm square holes. Both use provisional 2.50 mm pitch. Start with two dots,
insert all tails together, and check whether the normal plastic underside
supports can seat without bending pins. Then try the one-dot row.

The measured pin width is 0.64 mm; its other cross-section dimension remains
unconfirmed. Report which row accepts each header. Loose printed holes cannot
distinguish 2.50 from 2.54 mm pitch or validate a final PCB drill size. This is
a gross fit check, not an electrical adapter board. No wiring or power is needed.

Final support height, PCB footprint, hook access and strain relief remain
separate design steps. The generator checks manifold status, clear cutters,
watertightness, winding, positive volume and the expected separate body count.

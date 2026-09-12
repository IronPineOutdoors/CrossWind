# Controller connector: measured housings and fit gauges

User's existing Alpha controller harness has two six-position joined connections
and one four-position joined connection, each using a middle pin header. Thirteen
positions are used and three reserved. Preserve the existing wire assignments;
this is not a new IPC-101 pinout or a change to its separate STOP interface.

Kit: ELEAD B0G2L3LGWD, marketed as XH-style/Dupont assortment. Do not infer genuine
JST dimensional compatibility or wire-to-wire contact engagement from the title.

## Measurements received 2026-09-11

| Feature | mm | Evidence / interpretation |
| --- | ---: | --- |
| Six-position housing width | 15.97 | Explicitly confirmed by user, photo 184505 |
| Housing depth | 7.91 | Photo 185052; interpreted as mating-axis body depth |
| Six-position housing thickness | 3.98 | Photo 185114 |
| Header feature | 5.53 | Photo 185130; not used as assembled-pair length |
| Four-position housing width | 10.65 | Photo 185203 |
| Four-position housing thickness | 3.85 | Photo 185220 |
| Lip feature | 0.77 | Photo 185248; precise contact faces not established, not used yet |
| Rib inset from each housing end | 2.51 | User confirmed nearest edge; both connector sizes, wire side |
| Rib width | 0.71 | User confirmed; two ribs per housing |
| Maximum hooked-tip width | 1.28 | User confirmed 2026-09-12 after all three gauges caught |
| Rib projection from flat body | 0.79 | User confirmed after first fit print |

The complete connected pair length, header engagement, and retaining-shoulder
geometry are not established. The full carrier is pending these dimensions; no
claim of a finished 16-position connector is made by these gauges.

## Revised print: hooked-rib clearance (2026-09-12)

The first rectangular apertures caught on the two underside ribs. Regenerated
STLs now include two through-notches per opening, on the edge away from the
raised identification dots. Orient the connector ribs toward those notches.
The filenames replace the original rectangular gauges; reload the revised STL
and slice again rather than reusing the previous print job.

All three previous gauges failed to align in the physical fit check. The new
close-up shows angled ribs with wider hooked tips; the user measured the maximum
tip width as 1.28 mm, versus the 0.71 mm shaft used previously.

Rib locations retain the measured 2.51 mm shaft inset. Because the lateral tip
offset and rib angle are unmeasured, this diagnostic revision allows the extra
0.57 mm on BOTH sides of each shaft, making a 1.85 mm swept envelope before
clearance. This is a conservative test allowance, not a measured hook profile.
Notch widths are now 2.05, 2.25, and 2.45 mm for one, two, and three dots.
The same allowance is provisionally used for both housing sizes; verify each.
Each notch still extends 0.79 mm beyond the rectangular aperture edge, preserving
the same per-side clearance at the rib tip as at the flat body. Hook projection
has not been remeasured. The main rectangular openings are unchanged.
Reload and reslice the regenerated STLs; dot counts and filenames are unchanged.

Print `Connector_Fit_All_Three_PRINT.stl` on the A1 in PETG, 0.4 mm nozzle,
0.2 mm layers, four walls, solid fill; flat as exported, no supports. Individual
STLs are provided if only one clearance needs repeating. The three gauges are
38 x 16 x 2 mm plus raised identification dots; the combined print is 38 x 58 mm.

Each gauge has a wide six-position opening and a narrower four-position opening.
The added clearance is TOTAL, so 0.4 mm means 0.2 mm per side for a centered part.

| Raised dots | Total added clearance | Six-position opening | Four-position opening |
| ---: | ---: | --- | --- |
| 1 | 0.2 mm | 16.17 x 4.18 mm | 10.85 x 4.05 mm |
| 2 | 0.4 mm | 16.37 x 4.38 mm | 11.05 x 4.25 mm |
| 3 | 0.6 mm | 16.57 x 4.58 mm | 11.25 x 4.45 mm |

With power removed, test loose housings by inserting the body gently into its
opening. Do not force a flange/latch through an opening intended for the body;
note whether it is the body or an external ridge that stops insertion. Remove
first-layer burrs before judging fit. Report the smallest gauge that slides
without force for EACH connector size. These are dimensional coupons, not
electrical housings, clamps or strain relief.

## Remaining design inputs

- Full joined-pair length from one housing's wire-entry face to the other's with
  the middle header fully installed; compare the six- and four-position pairs.
- Fit-gauge results and any flange/ridge obstruction.
- Final retention must leave the metal contacts fully engaged without pushing
  pins backward or relying on the wires for structural support.
- USB service port remains a separate pending task: ESP socket type, mounting
  location and chosen extension cable geometry have not yet been confirmed.

`generate_fit_gauges.py` verifies clear apertures, manifold status, watertightness,
consistent winding, positive volume and the expected number of separate solids.
Physical dimensions and connector retention require printed fit verification.

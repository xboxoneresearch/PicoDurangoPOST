from gen_code_c_structs import gen_cstructs

MARKER_START = "// START_POSTCODE_SYNC_INJECTION"
MARKER_END = "// END_POSTCODE_SYNC_INJECTION"

structs_def = gen_cstructs()

with open("src/codes.h", "rt") as f:
    codes_h = f.read()

start_offset = codes_h.index(MARKER_START) + len(MARKER_START)
end_offset = codes_h.index(MARKER_END)

section_before = codes_h[0:start_offset]
section_after = codes_h[end_offset:]

updated_codes_h = section_before + "\n" + structs_def + "\n" + section_after
with open("src/codes.h_new", "wt") as f:
    f.write(updated_codes_h)
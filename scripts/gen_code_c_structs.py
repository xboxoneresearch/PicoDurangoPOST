import csv
import requests

POSTCODES_CSV_URL = "https://raw.githubusercontent.com/xboxoneresearch/errorcodes/refs/heads/main/postcodes.csv"

STRUCT_TEMPLATE = (
"""
static PostCode {typ}Codes[] = {{
{codes}
}};
"""
)

def generate_c_code(typ: str, rows: dict) -> str:
    entries = []
    for row in rows:
        code = int(row["Code"], 16)
        name = row["Name"]
        is_error = row["IsError"]

        entries.append(f"\t{{{code:#06x}, \"{name}\", {is_error}}}")
    res = STRUCT_TEMPLATE.format(typ=typ, codes=",\n".join(entries))
    return res

def gen_cstructs():
    """
    Generate c structs from postcode csv

    - Downloads the most recent csv for git repo https://github.com/xboxoneresearch/errorcodes
    - Parses it
    - Assembles the c structures
    """

    resp = requests.get(POSTCODES_CSV_URL)
    resp.raise_for_status()

    codes = []
    csv_content = resp.content.decode("utf-8")
    reader = csv.DictReader(csv_content.splitlines(), restkey="rest")
    for row in reader:
        assert row["Console"] in ["ALL", "XOP", "XOS", "XOX", "XSS", "XSX"]
        assert row["Type"] in ["SMC", "SP", "CPU", "OS"]
        assert row["Code"].startswith("0x")
        assert "rest" not in row, "Row was parsed incorrectly, is the description quoted properly?"
        codes.append(row)
    del reader

    cstructs = ""
    code_dict = {}
    for typ in ["SMC", "SP", "CPU", "OS"]:
        code_dict[typ] = filter(lambda x: x["Type"] == typ, codes)
        cstruct = generate_c_code(typ, code_dict[typ])
        cstructs += cstruct

    return cstructs

if __name__ == "__main__":
    print(gen_cstructs())
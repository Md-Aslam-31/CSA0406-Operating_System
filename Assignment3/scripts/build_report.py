from pathlib import Path
import re
from xml.sax.saxutils import escape

from docx import Document
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.shared import Inches, Pt, RGBColor
from reportlab.lib import colors
from reportlab.lib.enums import TA_CENTER
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib.units import mm
from reportlab.platypus import (BaseDocTemplate, Frame, PageBreak, PageTemplate, Paragraph,
                                Image as RLImage, Preformatted, Spacer, Table, TableStyle)

BASE = Path(__file__).resolve().parent.parent
lines = (BASE / "REPORT.md").read_text(encoding="utf-8").splitlines()

def inline(text):
    text = text.replace("–", "-").replace("—", "-").replace("’", "'").replace("“", '"').replace("”", '"').replace("×", "x").replace("→", "->").replace("•", "-")
    text = escape(text)
    import re
    text = re.sub(r"`([^`]+)`", r'<font name="Courier">\1</font>', text)
    text = re.sub(r"\*\*([^*]+)\*\*", r"<b>\1</b>", text)
    text = re.sub(r"\*([^*]+)\*", r"<i>\1</i>", text)
    text = re.sub(r"\[([^]]+)\]\(([^)]+)\)", r'<link href="\2" color="#175cd3">\1</link>', text)
    return text

# Editable Word report.
doc = Document()
sec = doc.sections[0]
sec.top_margin = Inches(.65); sec.bottom_margin = Inches(.65)
sec.left_margin = Inches(.75); sec.right_margin = Inches(.75)
normal = doc.styles["Normal"]; normal.font.name = "Arial"; normal.font.size = Pt(9.5)
normal.paragraph_format.space_after = Pt(4)
for name, size, color in [("Title", 24, "12345B"), ("Heading 1", 17, "12345B"), ("Heading 2", 13, "175CD3"), ("Heading 3", 11, "175CD3")]:
    st=doc.styles[name]; st.font.name="Arial"; st.font.size=Pt(size); st.font.bold=True; st.font.color.rgb=RGBColor.from_string(color)
in_code=False; code=[]; table_lines=[]
def flush_table():
    global table_lines
    rows=[]
    for line in table_lines:
        cells=[x.strip() for x in line.strip().strip("|").split("|")]
        if cells and not all(set(c) <= set(":- ") for c in cells): rows.append(cells)
    if rows:
        width=max(map(len,rows)); t=doc.add_table(rows=0, cols=width); t.style="Light Shading Accent 1"
        for row in rows:
            c=t.add_row().cells
            for i in range(width): c[i].text=row[i] if i<len(row) else ""
    table_lines=[]
for line in lines:
    if line.strip() == "<!-- pagebreak -->":
        doc.add_page_break(); continue
    image_match=re.fullmatch(r"!\[([^]]*)\]\(([^)]+)\)",line.strip())
    if image_match:
        p=doc.add_paragraph(); p.alignment=WD_ALIGN_PARAGRAPH.CENTER
        p.add_run().add_picture(str(BASE/image_match.group(2)),width=Inches(6.0))
        cap=doc.add_paragraph(image_match.group(1)); cap.alignment=WD_ALIGN_PARAGRAPH.CENTER
        continue
    if line.startswith("```"):
        if in_code:
            p=doc.add_paragraph(); r=p.add_run("\n".join(code)); r.font.name="Consolas"; r.font.size=Pt(8); p.paragraph_format.left_indent=Inches(.18); code=[]; in_code=False
        else:
            flush_table(); in_code=True
        continue
    if in_code: code.append(line); continue
    if line.startswith("|"):
        table_lines.append(line); continue
    flush_table()
    if not line.strip(): continue
    if line.startswith("# "):
        p=doc.add_paragraph(style="Title" if len(doc.paragraphs)==0 else "Heading 1"); p.add_run(line[2:].replace("`", ""))
    elif line.startswith("## "): doc.add_heading(line[3:], level=1)
    elif line.startswith("### "): doc.add_heading(line[4:], level=2)
    elif line.startswith("- "):
        p=doc.add_paragraph(style="List Bullet"); p.add_run(line[2:].replace("`", ""))
    elif line.startswith("| "): pass
    else:
        p=doc.add_paragraph(); p.add_run(line.replace("**", "").replace("`", ""))
doc.save(BASE / "REPORT.docx")

# Matching paginated PDF with selectable text and repeated page footer.
styles=getSampleStyleSheet()
styles.add(ParagraphStyle(name="CoverTitle", parent=styles["Title"], fontName="Helvetica-Bold", fontSize=21, leading=26, textColor=colors.HexColor("#12345b"), alignment=TA_CENTER, spaceAfter=14))
styles.add(ParagraphStyle(name="H1x", parent=styles["Heading1"], fontSize=16, leading=20, textColor=colors.HexColor("#12345b"), spaceBefore=10, spaceAfter=7, keepWithNext=True))
styles.add(ParagraphStyle(name="H2x", parent=styles["Heading2"], fontSize=12.5, leading=16, textColor=colors.HexColor("#175cd3"), spaceBefore=8, spaceAfter=5, keepWithNext=True))
styles.add(ParagraphStyle(name="Bodyx", parent=styles["BodyText"], fontName="Helvetica", fontSize=8.6, leading=11.2, spaceAfter=3.5))
styles.add(ParagraphStyle(name="Smallx", parent=styles["BodyText"], fontName="Helvetica", fontSize=7.5, leading=9))
def footer(canvas, doc):
    canvas.saveState(); canvas.setStrokeColor(colors.HexColor("#d0d5dd")); canvas.line(18*mm, 14*mm, 192*mm, 14*mm)
    canvas.setFont("Helvetica", 8); canvas.setFillColor(colors.HexColor("#475467")); canvas.drawString(18*mm, 9*mm, "CSA0406 Operating Systems | Assignment 2 | Mohamed Aslam H")
    canvas.drawRightString(192*mm, 9*mm, f"Page {doc.page}"); canvas.restoreState()
pdf=BaseDocTemplate(str(BASE/"REPORT.pdf"),pagesize=A4,leftMargin=18*mm,rightMargin=18*mm,topMargin=17*mm,bottomMargin=20*mm,title="CSA0406 Assignment 2 - Mohamed Aslam H",author="Mohamed Aslam H")
pdf.addPageTemplates(PageTemplate(id="report",frames=[Frame(18*mm,20*mm,174*mm,260*mm,id="normal")],onPage=footer))
story=[]; in_code=False; code=[]; table_lines=[]
def flush_pdf_table():
    global table_lines
    rows=[]
    for line in table_lines:
        cells=[x.strip() for x in line.strip().strip("|").split("|")]
        if cells and not all(set(c) <= set(":- ") for c in cells): rows.append([Paragraph(inline(x),styles["Smallx"]) for x in cells])
    if rows:
        n=max(len(r) for r in rows); rows=[r+[Paragraph("",styles["Smallx"])]*(n-len(r)) for r in rows]
        t=Table(rows,colWidths=[174*mm/n]*n,repeatRows=1,hAlign="LEFT")
        t.setStyle(TableStyle([("BACKGROUND",(0,0),(-1,0),colors.HexColor("#eaf0f8")),("TEXTCOLOR",(0,0),(-1,0),colors.HexColor("#12345b")),("GRID",(0,0),(-1,-1),.35,colors.HexColor("#cbd5e1")),("VALIGN",(0,0),(-1,-1),"TOP"),("LEFTPADDING",(0,0),(-1,-1),5),("RIGHTPADDING",(0,0),(-1,-1),5),("TOPPADDING",(0,0),(-1,-1),4),("BOTTOMPADDING",(0,0),(-1,-1),4)])); story.extend([Spacer(1,3),t,Spacer(1,6)])
    table_lines=[]
for line in lines:
    if line.strip() == "<!-- pagebreak -->":
        story.append(PageBreak()); continue
    image_match=re.fullmatch(r"!\[([^]]*)\]\(([^)]+)\)",line.strip())
    if image_match:
        im=RLImage(str(BASE/image_match.group(2)),width=174*mm)
        if im.drawHeight>86*mm: im.drawHeight=86*mm; im.drawWidth=86*mm*im.imageWidth/im.imageHeight
        story.extend([im,Paragraph(inline(image_match.group(1)),styles["Smallx"]),Spacer(1,5)])
        continue
    if line.startswith("```"):
        if in_code: story.append(Preformatted("\n".join(code),ParagraphStyle("code",fontName="Courier",fontSize=7.2,leading=9,backColor=colors.HexColor("#f2f4f7"),borderPadding=5))); story.append(Spacer(1,5)); code=[]; in_code=False
        else: flush_pdf_table(); in_code=True
        continue
    if in_code: code.append(line); continue
    if line.startswith("|"): table_lines.append(line); continue
    flush_pdf_table()
    if not line.strip(): story.append(Spacer(1,2)); continue
    if line.startswith("# "):
        story.append(Paragraph(inline(line[2:]),styles["CoverTitle"] if not story else styles["H1x"]))
    elif line.startswith("## "): story.append(Paragraph(inline(line[3:]),styles["H1x"]))
    elif line.startswith("### "): story.append(Paragraph(inline(line[4:]),styles["H2x"]))
    elif line.startswith("- "): story.append(Paragraph("- " + inline(line[2:]),styles["Bodyx"]))
    else: story.append(Paragraph(inline(line),styles["Bodyx"]))
flush_pdf_table(); pdf.build(story)
print(f"Built {BASE/'REPORT.docx'} and {BASE/'REPORT.pdf'}")

<?xml version="1.0"?>
<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns="http://www.w3.org/TR/xhtml1/strict">

<xsl:output
   method="html"
   indent="yes"
   encoding="utf-8"
/>

<xsl:template match="requirements">
 <html>
  <head>
   <title>
    <xsl:value-of select="title"/>
   </title>
  </head>
  <body>
   <xsl:apply-templates/>
  </body>
 </html>
</xsl:template>

<xsl:template match="title">
 <h1>
  <xsl:apply-templates/>
 </h1>
</xsl:template>

<xsl:template match="intro">
 <h2>Introduction</h2>
 <xsl:apply-templates/>
</xsl:template>

<xsl:template match="glossary">
 <h2>Glossary</h2>
 <ul>
  <xsl:apply-templates/>
 </ul>
</xsl:template>

<xsl:template match="term">
 <li>
  <xsl:apply-templates/>
 </li>
</xsl:template>

<xsl:template match="p">
 <p>
  <xsl:apply-templates/>
 </p>
</xsl:template>

<xsl:template match="reqtable">
 <h2>Requirements</h2>
 <table border="1">
 <tr>
  <th>ID</th>
  <th style="width: 75%">Requirement</th>
  <th style="width: 11%">Source</th>
  <th style="width: 11%">Implementation</th>
  </tr>
  <xsl:apply-templates/>
 </table>
</xsl:template>

<xsl:template match="req">
 <tr>
  <xsl:apply-templates/>
 </tr>
</xsl:template>

<xsl:template match="id">
 <td>
  <xsl:apply-templates/>
 </td>
</xsl:template>

<xsl:template match="category">
 <td colspan="3" style="font-weight: bold">
  <xsl:apply-templates/>
 </td>
</xsl:template>

<xsl:template match="text|source|impl">
 <td>
  <xsl:apply-templates/>
 </td>
</xsl:template>

<xsl:template match="emph">
 <em>
  <xsl:apply-templates/>
 </em>
</xsl:template>

<xsl:template match="strong">
 <strong>
  <xsl:apply-templates/>
 </strong>
</xsl:template>

</xsl:stylesheet>

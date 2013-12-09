<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output  omit-xml-declaration="yes" method="xml" indent="yes"/>
<xsl:preserve-space elements="para"/>
<xsl:template match="Messages">
<section id="rxsubcomup">
<title>RXSUBCOM Utility Program</title>
<xsl:text>
</xsl:text>
<para>RXSUBCOM issues the following errors:</para>
<xsl:text>
</xsl:text>

<xsl:for-each select="Message/Subcodes/SubMessage[Component = 'RXSUBCOM']">
<xsl:sort select="MessageNumber" data-type="number"/>
<xsl:text>
</xsl:text>
<xsl:element name="section"> <xsl:attribute name="id">ERR<xsl:value-of select="MessageNumber"/></xsl:attribute>
<title>Error <xsl:value-of select="MessageNumber"/> - <xsl:apply-templates select="Text"/></title>
<xsl:choose>
<xsl:when test="Explanation">
<xsl:apply-templates select="Explanation"/>
</xsl:when>
<xsl:otherwise>
<para><xsl:text> </xsl:text></para>
</xsl:otherwise>
</xsl:choose>
</xsl:element>
</xsl:for-each>

</section>
</xsl:template>


<xsl:template match="Text">
    <xsl:apply-templates/>
</xsl:template>

<xsl:template match="q">
<xsl:text>&quot;</xsl:text><xsl:apply-templates/><xsl:text>&quot;</xsl:text>
</xsl:template>

<xsl:template match="sq">
<xsl:text>&apos;</xsl:text>
</xsl:template>

<xsl:template match="dq">
<xsl:text>&quot;</xsl:text>
</xsl:template>

<xsl:template match="Sub">
<emphasis role="italic"><xsl:value-of select="@name"/></emphasis>
</xsl:template>

<xsl:template match="Explanation">
<xsl:text>
</xsl:text>
<para><emphasis role="bold">Explanation:</emphasis></para>
<xsl:text>
</xsl:text>
<xsl:copy-of select="*"/>
</xsl:template>

</xsl:stylesheet>

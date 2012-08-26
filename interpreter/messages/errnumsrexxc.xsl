<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output  omit-xml-declaration="yes" method="xml" indent="yes"/>
<xsl:preserve-space elements="para"/>
<xsl:template match="Messages"><xsl:text disable-output-escaping="yes"><![CDATA[<?xml version="1.0"?>]]></xsl:text><xsl:text>
</xsl:text><xsl:text disable-output-escaping="yes"><![CDATA[<!DOCTYPE section PUBLIC "-//OASIS//DTD DocBook XML V4.5//EN" "http://www.oasis-open.org/docbook/xml/4.5/docbookx.dtd" []]></xsl:text><xsl:text>
</xsl:text><xsl:text disable-output-escaping="yes"><![CDATA[<!ENTITY % BOOK_ENTITIES SYSTEM "rexxref.ent">]]></xsl:text><xsl:text>
</xsl:text><xsl:text disable-output-escaping="yes"><![CDATA[%BOOK_ENTITIES;]]></xsl:text><xsl:text>
</xsl:text><xsl:text disable-output-escaping="yes"><![CDATA[]>]]></xsl:text><xsl:text>
</xsl:text>
<xsl:comment>###########################################################################</xsl:comment><xsl:text>
</xsl:text><xsl:comment>#                                                                          </xsl:comment><xsl:text>
</xsl:text><xsl:comment># Description: Open Object Rexx: Reference XML File                        </xsl:comment><xsl:text>
</xsl:text><xsl:comment>#                                                                          </xsl:comment><xsl:text>
</xsl:text><xsl:comment># Copyright (c) 2005-2012, Rexx Language Association. All rights reserved. </xsl:comment><xsl:text>
</xsl:text><xsl:comment># Portions Copyright (c) 2004, IBM Corporation. All rights reserved.       </xsl:comment><xsl:text>
</xsl:text><xsl:comment>#                                                                          </xsl:comment><xsl:text>
</xsl:text><xsl:comment># This program and the accompanying materials are made available under     </xsl:comment><xsl:text>
</xsl:text><xsl:comment># the terms of the Common Public License v1.0 which accompanies this       </xsl:comment><xsl:text>
</xsl:text><xsl:comment># distribution. A copy is also available at the following address:         </xsl:comment><xsl:text>
</xsl:text><xsl:comment># http://www.oorexx.org/license.html                                       </xsl:comment><xsl:text>
</xsl:text><xsl:comment>#                                                                          </xsl:comment><xsl:text>
</xsl:text><xsl:comment># Redistribution and use in source and binary forms, with or               </xsl:comment><xsl:text>
</xsl:text><xsl:comment># without modification, are permitted provided that the following          </xsl:comment><xsl:text>
</xsl:text><xsl:comment># conditions are met:                                                      </xsl:comment><xsl:text>
</xsl:text><xsl:comment>#                                                                          </xsl:comment><xsl:text>
</xsl:text><xsl:comment># Redistributions of source code must retain the above copyright           </xsl:comment><xsl:text>
</xsl:text><xsl:comment># notice, this list of conditions and the following disclaimer.            </xsl:comment><xsl:text>
</xsl:text><xsl:comment># Redistributions in binary form must reproduce the above copyright        </xsl:comment><xsl:text>
</xsl:text><xsl:comment># notice, this list of conditions and the following disclaimer in          </xsl:comment><xsl:text>
</xsl:text><xsl:comment># the documentation and/or other materials provided with the distribution. </xsl:comment><xsl:text>
</xsl:text><xsl:comment>#                                                                          </xsl:comment><xsl:text>
</xsl:text><xsl:comment># Neither the name of Rexx Language Association nor the names              </xsl:comment><xsl:text>
</xsl:text><xsl:comment># of its contributors may be used to endorse or promote products           </xsl:comment><xsl:text>
</xsl:text><xsl:comment># derived from this software without specific prior written permission.    </xsl:comment><xsl:text>
</xsl:text><xsl:comment>#                                                                          </xsl:comment><xsl:text>
</xsl:text><xsl:comment># THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS      </xsl:comment><xsl:text>
</xsl:text><xsl:comment># "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT        </xsl:comment><xsl:text>
</xsl:text><xsl:comment># LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS        </xsl:comment><xsl:text>
</xsl:text><xsl:comment># FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT </xsl:comment><xsl:text>
</xsl:text><xsl:comment># OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,    </xsl:comment><xsl:text>
</xsl:text><xsl:comment># SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED </xsl:comment><xsl:text>
</xsl:text><xsl:comment># TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,      </xsl:comment><xsl:text>
</xsl:text><xsl:comment># OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY   </xsl:comment><xsl:text>
</xsl:text><xsl:comment># OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING  </xsl:comment><xsl:text>
</xsl:text><xsl:comment># NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS       </xsl:comment><xsl:text>
</xsl:text><xsl:comment># SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.             </xsl:comment><xsl:text>
</xsl:text><xsl:comment>#                                                                          </xsl:comment><xsl:text>
</xsl:text><xsl:comment>###########################################################################</xsl:comment><xsl:text>
</xsl:text>
<section id="rexxcup">
<title>RexxC Utility Program</title>
<para>When RexxC encounters a syntax error in a Rexx program while tokenizing or syntax checking it, RexxC
returns the negated ooRexx error code. In addition, RexxC issues the following errors:</para>

<xsl:for-each select="Message/Subcodes/SubMessage[Component = 'REXXC']">
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

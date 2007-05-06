@echo off
@rem --------------------------------------------------------------------------*
@rem
@rem Copyright (c) 2007 Rexx Language Association. All rights reserved.
@rem
@rem This program and the accompanying materials are made available under
@rem the terms of the Common Public License v1.0 which accompanies this
@rem distribution. A copy is also available at the following address:
@rem http://www.oorexx.org/license.html
@rem
@rem Redistribution and use in source and binary forms, with or
@rem without modification, are permitted provided that the following
@rem conditions are met:
@rem
@rem Redistributions of source code must retain the above copyright
@rem notice, this list of conditions and the following disclaimer.
@rem Redistributions in binary form must reproduce the above copyright
@rem notice, this list of conditions and the following disclaimer in
@rem the documentation and/or other materials provided with the distribution.
@rem
@rem Neither the name of Rexx Language Association nor the names
@rem of its contributors may be used to endorse or promote products
@rem derived from this software without specific prior written permission.
@rem
@rem THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
@rem "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
@rem LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
@rem FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
@rem OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
@rem SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
@rem TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
@rem OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
@rem OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
@rem NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
@rem SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
@rem
@rem --------------------------------------------------------------------------*

for /F "eol=# tokens=2*" %%i in (touchheaders.sh) do stouch %%i

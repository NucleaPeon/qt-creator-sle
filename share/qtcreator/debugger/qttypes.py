############################################################################
#
# Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
# Contact: http://www.qt-project.org/legal
#
# This file is part of Qt Creator.
#
# Commercial License Usage
# Licensees holding valid commercial Qt licenses may use this file in
# accordance with the commercial license agreement provided with the
# Software or, alternatively, in accordance with the terms contained in
# a written agreement between you and Digia.  For licensing terms and
# conditions see http://qt.digia.com/licensing.  For further information
# use the contact form at http://qt.digia.com/contact-us.
#
# GNU Lesser General Public License Usage
# Alternatively, this file may be used under the terms of the GNU Lesser
# General Public License version 2.1 as published by the Free Software
# Foundation and appearing in the file LICENSE.LGPL included in the
# packaging of this file.  Please review the following information to
# ensure the GNU Lesser General Public License version 2.1 requirements
# will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
#
# In addition, as a special exception, Digia gives you certain additional
# rights.  These rights are described in the Digia Qt LGPL Exception
# version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
#
#############################################################################

import platform
from dumper import *


def qdump__QAtomicInt(d, value):
    d.putValue(int(value["_q_value"]))
    d.putNumChild(0)


def qdump__QBasicAtomicInt(d, value):
    d.putValue(int(value["_q_value"]))
    d.putNumChild(0)


def qdump__QAtomicPointer(d, value):
    d.putType(value.type)
    q = value["_q_value"]
    p = toInteger(q)
    d.putValue("@0x%x" % p)
    d.putNumChild(1 if p else 0)
    if d.isExpanded():
        with Children(d):
           d.putSubItem("_q_value", q.dereference())

def qform__QByteArray():
    return "Inline,As Latin1 in Separate Window,As UTF-8 in Separate Window"

def qdump__QByteArray(d, value):
    d.putByteArrayValue(value)
    data, size, alloc = d.byteArrayData(value)
    d.putNumChild(size)
    format = d.currentItemFormat()
    if format == 1:
        d.putDisplay(StopDisplay)
    elif format == 2:
        d.putField("editformat", DisplayLatin1String)
        d.putField("editvalue", d.encodeByteArray(value))
    elif format == 3:
        d.putField("editformat", DisplayUtf8String)
        d.putField("editvalue", d.encodeByteArray(value))
    if d.isExpanded():
        d.putArrayData(d.charType(), data, size)

def qdump__QByteArrayData(d, value):
    data, size, alloc = d.byteArrayDataHelper(d.addressOf(value))
    d.putValue(d.readMemory(data, size), Hex2EncodedLatin1)
    d.putNumChild(1)
    if d.isExpanded():
        with Children(d):
            d.putIntItem("size", size)
            d.putIntItem("alloc", alloc)

def qdump__QChar(d, value):
    d.putValue(int(value["ucs"]))
    d.putNumChild(0)


def qform__QAbstractItemModel():
    return "Normal,Enhanced"

def qdump__QAbstractItemModel(d, value):
    format = d.currentItemFormat()
    if format == 1:
        d.putPlainChildren(value)
        return
    #format == 2:
    # Create a default-constructed QModelIndex on the stack.
    try:
        ri = d.makeValue(d.qtNamespace() + "QModelIndex", "-1, -1, 0, 0")
        this_ = d.makeExpression(value)
        ri_ = d.makeExpression(ri)
        rowCount = int(d.parseAndEvaluate("%s.rowCount(%s)" % (this_, ri_)))
        columnCount = int(d.parseAndEvaluate("%s.columnCount(%s)" % (this_, ri_)))
    except:
        d.putPlainChildren(value)
        return
    d.putValue("%d x %d" % (rowCount, columnCount))
    d.putNumChild(rowCount * columnCount)
    if d.isExpanded():
        with Children(d, numChild=rowCount * columnCount, childType=ri.type):
            i = 0
            for row in xrange(rowCount):
                for column in xrange(columnCount):
                    with SubItem(d, i):
                        d.putName("[%s, %s]" % (row, column))
                        mi = d.parseAndEvaluate("%s.index(%d,%d,%s)"
                            % (this_, row, column, ri_))
                        #warn("MI: %s " % mi)
                        #name = "[%d,%d]" % (row, column)
                        #d.putValue("%s" % mi)
                        d.putItem(mi)
                        i = i + 1
                        #warn("MI: %s " % mi)
                        #d.putName("[%d,%d]" % (row, column))
                        #d.putValue("%s" % mi)
                        #d.putNumChild(0)
                        #d.putType(mi.type)
    #gdb.execute("call free($ri)")

def qform__QModelIndex():
    return "Normal,Enhanced"

def qdump__QModelIndex(d, value):
    format = d.currentItemFormat()
    if format == 1:
        d.putPlainChildren(value)
        return
    r = value["r"]
    c = value["c"]
    try:
        p = value["p"]
    except:
        p = value["i"]
    m = value["m"]
    if d.isNull(m) or r < 0 or c < 0:
        d.putValue("(invalid)")
        d.putPlainChildren(value)
        return

    mm = m.dereference()
    mm = mm.cast(mm.type.unqualified())
    ns = d.qtNamespace()
    try:
        mi = d.makeValue(ns + "QModelIndex", "%s,%s,%s,%s" % (r, c, p, m))
        mm_ = d.makeExpression(mm)
        mi_ = d.makeExpression(mi)
        rowCount = int(d.parseAndEvaluate("%s.rowCount(%s)" % (mm_, mi_)))
        columnCount = int(d.parseAndEvaluate("%s.columnCount(%s)" % (mm_, mi_)))
    except:
        d.putPlainChildren(value)
        return

    try:
        # Access DisplayRole as value
        val = d.parseAndEvaluate("%s.data(%s, 0)" % (mm_, mi_))
        v = val["d"]["data"]["ptr"]
        d.putStringValue(d.makeValue(ns + 'QString', v))
    except:
        d.putValue("")

    d.putNumChild(rowCount * columnCount)
    if d.isExpanded():
        with Children(d):
            i = 0
            for row in xrange(rowCount):
                for column in xrange(columnCount):
                    with UnnamedSubItem(d, i):
                        d.putName("[%s, %s]" % (row, column))
                        mi2 = d.parseAndEvaluate("%s.index(%d,%d,%s)"
                            % (mm_, row, column, mi_))
                        d.putItem(mi2)
                        i = i + 1
            d.putFields(value)
            #d.putCallItem("parent", val, "parent")
            #with SubItem(d, "model"):
            #    d.putValue(m)
            #    d.putType(ns + "QAbstractItemModel*")
            #    d.putNumChild(1)
    #gdb.execute("call free($mi)")


def qdump__QDate(d, value):
    jd = int(value["jd"])
    if jd:
        d.putValue(jd, JulianDate)
        d.putNumChild(1)
        if d.isExpanded():
            # FIXME: This improperly uses complex return values.
            with Children(d):
                d.putCallItem("toString", value, "toString",
                    d.enumExpression("DateFormat", "TextDate"))
                d.putCallItem("(ISO)", value, "toString",
                    d.enumExpression("DateFormat", "ISODate"))
                d.putCallItem("(SystemLocale)", value, "toString",
                    d.enumExpression("DateFormat", "SystemLocaleDate"))
                d.putCallItem("(Locale)", value, "toString",
                    d.enumExpression("DateFormat", "LocaleDate"))
                d.putFields(value)
    else:
        d.putValue("(invalid)")
        d.putNumChild(0)


def qdump__QTime(d, value):
    mds = int(value["mds"])
    if mds >= 0:
        d.putValue(mds, MillisecondsSinceMidnight)
        d.putNumChild(1)
        if d.isExpanded():
            # FIXME: This improperly uses complex return values.
            with Children(d):
                d.putCallItem("toString", value, "toString",
                    d.enumExpression("DateFormat", "TextDate"))
                d.putCallItem("(ISO)", value, "toString",
                    d.enumExpression("DateFormat", "ISODate"))
                d.putCallItem("(SystemLocale)", value, "toString",
                    d.enumExpression("DateFormat", "SystemLocaleDate"))
                d.putCallItem("(Locale)", value, "toString",
                    d.enumExpression("DateFormat", "LocaleDate"))
                d.putFields(value)
    else:
        d.putValue("(invalid)")
        d.putNumChild(0)


def qdump__QTimeZone(d, value):
    base = d.extractPointer(value)
    if base == 0:
        d.putValue("(null)")
        d.putNumChild(0)
        return
    idAddr = base + 2 * d.ptrSize() # [QSharedData] + [vptr]
    d.putByteArrayValueByAddress(idAddr)
    d.putPlainChildren(value["d"])


def qdump__QDateTime(d, value):
    qtVersion = d.qtVersion()
    isValid = False
    # This relies on the Qt4/Qt5 internal structure layout:
    # {sharedref(4), ...
    base = d.extractPointer(value)
    is32bit = d.is32bit()
    if qtVersion >= 0x050200:
        if d.isWindowsTarget():
            msecsOffset = 8
            specOffset = 16
            offsetFromUtcOffset = 20
            timeZoneOffset = 24
            statusOffset = 28 if is32bit else 32
        else:
            msecsOffset = 4 if is32bit else 8
            specOffset = 12 if is32bit else 16
            offsetFromUtcOffset = 16 if is32bit else 20
            timeZoneOffset = 20 if is32bit else 24
            statusOffset = 24 if is32bit else 32
        status = d.extractInt(base + statusOffset)
        if int(status & 0x0c == 0x0c): # ValidDate and ValidTime
            isValid = True
            msecs = d.extractInt64(base + msecsOffset)
            spec = d.extractInt(base + specOffset)
            offset = d.extractInt(base + offsetFromUtcOffset)
            tzp = d.extractPointer(base + timeZoneOffset)
            if tzp == 0:
                tz = ""
            else:
                idBase = tzp + 2 * d.ptrSize() # [QSharedData] + [vptr]
                tz = d.encodeByteArrayHelper(d.extractPointer(idBase))
            d.putValue("%s/%s/%s/%s/%s" % (msecs, spec, offset, tz, status),
                DateTimeInternal)
    else:
        # This relies on the Qt4/Qt5 internal structure layout:
        # {sharedref(4), date(8), time(4+x)}
        # QDateTimePrivate:
        # - QAtomicInt ref;    (padded on 64 bit)
        # -     [QDate date;]
        # -      -  uint jd in Qt 4,  qint64 in Qt 5.0 and Qt 5.1; padded on 64 bit
        # -     [QTime time;]
        # -      -  uint mds;
        # -  Spec spec;
        dateSize = 8 if qtVersion >= 0x050000 else 4 # Qt5: qint64, Qt4 uint
        # 4 byte padding after 4 byte QAtomicInt if we are on 64 bit and QDate is 64 bit
        refPlusPadding = 8 if qtVersion >= 0x050000 and not d.is32bit() else 4
        dateBase = base + refPlusPadding
        timeBase = dateBase + dateSize
        mds = d.extractInt(timeBase)
        isValid = mds > 0
        if isValid:
            jd = d.extractInt(dateBase)
            d.putValue("%s/%s" % (jd, mds), JulianDateAndMillisecondsSinceMidnight)
    if isValid:
        d.putNumChild(1)
        if d.isExpanded():
            # FIXME: This improperly uses complex return values.
            with Children(d):
                d.putCallItem("toTime_t", value, "toTime_t")
                d.putCallItem("toString", value, "toString",
                    d.enumExpression("DateFormat", "TextDate"))
                d.putCallItem("(ISO)", value, "toString",
                    d.enumExpression("DateFormat", "ISODate"))
                d.putCallItem("(SystemLocale)", value, "toString",
                    d.enumExpression("DateFormat", "SystemLocaleDate"))
                d.putCallItem("(Locale)", value, "toString",
                    d.enumExpression("DateFormat", "LocaleDate"))
                d.putCallItem("toUTC", value, "toTimeSpec",
                    d.enumExpression("TimeSpec", "UTC"))
                d.putCallItem("toLocalTime", value, "toTimeSpec",
                    d.enumExpression("TimeSpec", "LocalTime"))
                d.putFields(value)
    else:
        d.putValue("(invalid)")
        d.putNumChild(0)


def qdump__QDir(d, value):
    d.putNumChild(1)
    privAddress = d.extractPointer(value)
    bit32 = d.is32bit()
    qt5 = d.qtVersion() >= 0x050000

    # Change 9fc0965 reorders members again.
    #  bool fileListsInitialized;\n"
    #  QStringList files;\n"
    #  QFileInfoList fileInfos;\n"
    #  QStringList nameFilters;\n"
    #  QDir::SortFlags sort;\n"
    #  QDir::Filters filters;\n"

    # Before 9fc0965:
    # QDirPrivate:
    # QAtomicInt ref
    # QStringList nameFilters;
    # QDir::SortFlags sort;
    # QDir::Filters filters;
    # // qt3support:
    # QChar filterSepChar;
    # bool matchAllDirs;
    # // end qt3support
    # QScopedPointer<QAbstractFileEngine> fileEngine;
    # bool fileListsInitialized;
    # QStringList files;
    # QFileInfoList fileInfos;
    # QFileSystemEntry dirEntry;
    # QFileSystemEntry absoluteDirEntry;

    # QFileSystemEntry:
    # QString m_filePath
    # QByteArray m_nativeFilePath
    # qint16 m_lastSeparator
    # qint16 m_firstDotInFileName
    # qint16 m_lastDotInFileName
    # + 2 byte padding
    fileSystemEntrySize = 2 * d.ptrSize() + 8

    if d.qtVersion() < 0x050200:
        case = 0
    elif d.qtVersion() >= 0x050300:
        case = 1
    else:
        # Try to distinguish bool vs QStringList at the first item
        # after the (padded) refcount. If it looks like a bool assume
        # this is after 9fc0965. This is not safe.
        firstValue = d.extractInt(privAddress + d.ptrSize())
        case = 1 if firstValue == 0 or firstValue == 1 else 0

    if case == 1:
        if bit32:
            filesOffset = 4
            fileInfosOffset = 8
            dirEntryOffset = 0x20
            absoluteDirEntryOffset = 0x30
        else:
            filesOffset = 0x08
            fileInfosOffset = 0x10
            dirEntryOffset = 0x30
            absoluteDirEntryOffset = 0x48
    else:
        # Assume this is before 9fc0965.
        qt3support = d.isQt3Support()
        qt3SupportAddition = d.ptrSize() if qt3support else 0
        filesOffset = (24 if bit32 else 40) + qt3SupportAddition
        fileInfosOffset = filesOffset + d.ptrSize()
        dirEntryOffset = fileInfosOffset + d.ptrSize()
        absoluteDirEntryOffset = dirEntryOffset + fileSystemEntrySize

    d.putStringValueByAddress(privAddress + dirEntryOffset)
    if d.isExpanded():
        with Children(d):
            ns = d.qtNamespace()
            d.call(value, "count")  # Fill cache.
            #d.putCallItem("absolutePath", value, "absolutePath")
            #d.putCallItem("canonicalPath", value, "canonicalPath")
            with SubItem(d, "absolutePath"):
                typ = d.lookupType(ns + "QString")
                d.putItem(d.createValue(privAddress + absoluteDirEntryOffset, typ))
            with SubItem(d, "entryInfoList"):
                typ = d.lookupType(ns + "QList<" + ns + "QFileInfo>")
                d.putItem(d.createValue(privAddress + fileInfosOffset, typ))
            with SubItem(d, "entryList"):
                typ = d.lookupType(ns + "QStringList")
                d.putItem(d.createValue(privAddress + filesOffset, typ))
            d.putFields(value)


def qdump__QFile(d, value):
    # 9fc0965 changes the layout of the private structure
    qtVersion = d.qtVersion()
    is32bit = d.is32bit()
    if qtVersion > 0x050200:
        if d.isWindowsTarget():
            offset = 180 if is32bit else 272
        else:
            offset = 176 if is32bit else 272
    elif qtVersion >= 0x050000:
        offset = 176 if is32bit else 280
    else:
        if d.isWindowsTarget():
            offset = 144 if is32bit else 232
        else:
            offset = 140 if is32bit else 232
    privAddress = d.extractPointer(d.addressOf(value) + d.ptrSize())
    fileNameAddress = privAddress + offset
    d.putStringValueByAddress(fileNameAddress)
    d.putNumChild(1)
    if d.isExpanded():
        with Children(d):
            d.putCallItem("exists", value, "exists")
            d.putFields(value)


def qdump__QFileInfo(d, value):
    privAddress = d.extractPointer(value)
    #bit32 = d.is32bit()
    #qt5 = d.qtVersion() >= 0x050000
    #try:
    #    d.putStringValue(value["d_ptr"]["d"].dereference()["fileNames"][3])
    #except:
    #    d.putPlainChildren(value)
    #    return
    filePathAddress = privAddress + d.ptrSize()
    d.putStringValueByAddress(filePathAddress)
    d.putNumChild(1)
    if d.isExpanded():
        ns = d.qtNamespace()
        with Children(d, childType=d.lookupType(ns + "QString")):
            d.putCallItem("absolutePath", value, "absolutePath")
            d.putCallItem("absoluteFilePath", value, "absoluteFilePath")
            d.putCallItem("canonicalPath", value, "canonicalPath")
            d.putCallItem("canonicalFilePath", value, "canonicalFilePath")
            d.putCallItem("completeBaseName", value, "completeBaseName")
            d.putCallItem("completeSuffix", value, "completeSuffix")
            d.putCallItem("baseName", value, "baseName")
            if False:
                #ifdef Q_OS_MACX
                d.putCallItem("isBundle", value, "isBundle")
                d.putCallItem("bundleName", value, "bundleName")
            d.putCallItem("fileName", value, "fileName")
            d.putCallItem("filePath", value, "filePath")
            # Crashes gdb (archer-tromey-python, at dad6b53fe)
            #d.putCallItem("group", value, "group")
            #d.putCallItem("owner", value, "owner")
            d.putCallItem("path", value, "path")

            d.putCallItem("groupid", value, "groupId")
            d.putCallItem("ownerid", value, "ownerId")

            #QFile::Permissions permissions () const
            perms = d.call(value, "permissions")
            if perms is None:
                d.putValue("<not available>")
            else:
                with SubItem(d, "permissions"):
                    d.putEmptyValue()
                    d.putType(ns + "QFile::Permissions")
                    d.putNumChild(10)
                    if d.isExpanded():
                        with Children(d, 10):
                            perms = perms['i']
                            d.putBoolItem("ReadOwner",  perms & 0x4000)
                            d.putBoolItem("WriteOwner", perms & 0x2000)
                            d.putBoolItem("ExeOwner",   perms & 0x1000)
                            d.putBoolItem("ReadUser",   perms & 0x0400)
                            d.putBoolItem("WriteUser",  perms & 0x0200)
                            d.putBoolItem("ExeUser",    perms & 0x0100)
                            d.putBoolItem("ReadGroup",  perms & 0x0040)
                            d.putBoolItem("WriteGroup", perms & 0x0020)
                            d.putBoolItem("ExeGroup",   perms & 0x0010)
                            d.putBoolItem("ReadOther",  perms & 0x0004)
                            d.putBoolItem("WriteOther", perms & 0x0002)
                            d.putBoolItem("ExeOther",   perms & 0x0001)

            #QDir absoluteDir () const
            #QDir dir () const
            d.putCallItem("caching", value, "caching")
            d.putCallItem("exists", value, "exists")
            d.putCallItem("isAbsolute", value, "isAbsolute")
            d.putCallItem("isDir", value, "isDir")
            d.putCallItem("isExecutable", value, "isExecutable")
            d.putCallItem("isFile", value, "isFile")
            d.putCallItem("isHidden", value, "isHidden")
            d.putCallItem("isReadable", value, "isReadable")
            d.putCallItem("isRelative", value, "isRelative")
            d.putCallItem("isRoot", value, "isRoot")
            d.putCallItem("isSymLink", value, "isSymLink")
            d.putCallItem("isWritable", value, "isWritable")
            d.putCallItem("created", value, "created")
            d.putCallItem("lastModified", value, "lastModified")
            d.putCallItem("lastRead", value, "lastRead")
            d.putFields(value)


def qdump__QFixed(d, value):
    v = int(value["val"])
    d.putValue("%s/64 = %s" % (v, v/64.0))
    d.putNumChild(0)


def qform__QFiniteStack():
    return arrayForms()

def qdump__QFiniteStack(d, value):
    alloc = int(value["_alloc"])
    size = int(value["_size"])
    d.check(0 <= size and size <= alloc and alloc <= 1000 * 1000 * 1000)
    d.putItemCount(size)
    d.putNumChild(size)
    if d.isExpanded():
        innerType = d.templateArgument(value.type, 0)
        d.putPlotData(innerType, value["_array"], size)

# Stock gdb 7.2 seems to have a problem with types here:
#
#  echo -e "namespace N { struct S { enum E { zero, one, two }; }; }\n"\
#      "int main() { N::S::E x = N::S::one;\n return x; }" >> main.cpp
#  g++ -g main.cpp
#  gdb-7.2 -ex 'file a.out' -ex 'b main' -ex 'run' -ex 'step' \
#     -ex 'ptype N::S::E' -ex 'python print gdb.lookup_type("N::S::E")' -ex 'q'
#  gdb-7.1 -ex 'file a.out' -ex 'b main' -ex 'run' -ex 'step' \
#     -ex 'ptype N::S::E' -ex 'python print gdb.lookup_type("N::S::E")' -ex 'q'
#  gdb-cvs -ex 'file a.out' -ex 'b main' -ex 'run' -ex 'step' \
#     -ex 'ptype N::S::E' -ex 'python print gdb.lookup_type("N::S::E")' -ex 'q'
#
# gives as of 2010-11-02
#
#  type = enum N::S::E {N::S::zero, N::S::one, N::S::two} \n
#    Traceback (most recent call last): File "<string>", line 1,
#      in <module> RuntimeError: No type named N::S::E.
#  type = enum N::S::E {N::S::zero, N::S::one, N::S::two} \n  N::S::E
#  type = enum N::S::E {N::S::zero, N::S::one, N::S::two} \n  N::S::E
#
# i.e. there's something broken in stock 7.2 that is was ok in 7.1 and is ok later.

def qdump__QFlags(d, value):
    i = value["i"]
    try:
        enumType = d.templateArgument(value.type.unqualified(), 0)
        d.putValue("%s (%s)" % (i.cast(enumType), i))
    except:
        d.putValue("%s" % i)
    d.putNumChild(0)


def qform__QHash():
    return mapForms()

def qdump__QHash(d, value):

    def hashDataFirstNode(dPtr, numBuckets):
        ePtr = dPtr.cast(nodeTypePtr)
        bucket = dPtr.dereference()["buckets"]
        for n in xrange(numBuckets - 1, -1, -1):
            n = n - 1
            if n < 0:
                break
            if d.pointerValue(bucket.dereference()) != d.pointerValue(ePtr):
                return bucket.dereference()
            bucket = bucket + 1
        return ePtr;

    def hashDataNextNode(nodePtr, numBuckets):
        nextPtr = nodePtr.dereference()["next"]
        if d.pointerValue(nextPtr.dereference()["next"]):
            return nextPtr
        start = (int(nodePtr.dereference()["h"]) % numBuckets) + 1
        dPtr = nextPtr.cast(dataTypePtr)
        bucket = dPtr.dereference()["buckets"] + start
        for n in xrange(numBuckets - start):
            if d.pointerValue(bucket.dereference()) != d.pointerValue(nextPtr):
                return bucket.dereference()
            bucket += 1
        return nextPtr

    keyType = d.templateArgument(value.type, 0)
    valueType = d.templateArgument(value.type, 1)

    anon = d.childAt(value, 0)
    d_ptr = anon["d"]
    e_ptr = anon["e"]
    size = int(d_ptr["size"])

    dataTypePtr = d_ptr.type    # QHashData *  = { Node *fakeNext, Node *buckets }
    nodeTypePtr = d_ptr.dereference()["fakeNext"].type    # QHashData::Node

    d.check(0 <= size and size <= 100 * 1000 * 1000)
    d.checkRef(d_ptr["ref"])

    d.putItemCount(size)
    d.putNumChild(size)
    if d.isExpanded():
        numBuckets = int(d_ptr.dereference()["numBuckets"])
        innerType = e_ptr.dereference().type
        isCompact = d.isMapCompact(keyType, valueType)
        childType = valueType if isCompact else innerType
        with Children(d, size, maxNumChild=1000, childType=childType):
            for i in d.childRange():
                if i == 0:
                    node = hashDataFirstNode(d_ptr, numBuckets)
                else:
                    node = hashDataNextNode(node, numBuckets)
                it = node.dereference().cast(innerType)
                with SubItem(d, i):
                    if isCompact:
                        key = it["key"]
                        if not key:
                            # LLDB can't access directly since it's in anonymous union
                            # for Qt4 optimized int keytype
                            key = it[1]["key"]
                        d.putMapName(key)
                        d.putItem(it["value"])
                        d.putType(valueType)
                    else:
                        d.putItem(it)


def qdump__QHashNode(d, value):
    key = value["key"]
    if not key:
        # LLDB can't access directly since it's in anonymous union
        # for Qt4 optimized int keytype
        key = value[1]["key"]
    val = value["value"]
    d.putEmptyValue()
    d.putNumChild(2)
    if d.isExpanded():
        with Children(d):
            d.putSubItem("key", key)
            d.putSubItem("value", val)


def qHashIteratorHelper(d, value):
    typeName = str(value.type)
    hashTypeName = typeName[0:typeName.rfind("::")]
    hashType = d.lookupType(hashTypeName)
    keyType = d.templateArgument(hashType, 0)
    valueType = d.templateArgument(hashType, 1)
    d.putNumChild(1)
    d.putEmptyValue()
    if d.isExpanded():
        with Children(d):
            # We need something like QHash<int, float>::iterator
            # -> QHashNode<int, float> with 'proper' spacing,
            # as space changes confuse LLDB.
            innerTypeName = hashTypeName.replace("QHash", "QHashNode", 1)
            node = value["i"].cast(d.lookupType(innerTypeName).pointer()).dereference()
            key = node["key"]
            if not key:
                # LLDB can't access directly since it's in anonymous union
                # for Qt4 optimized int keytype
                key = node[1]["key"]
            d.putSubItem("key", key)
            d.putSubItem("value", node["value"])

def qdump__QHash__const_iterator(d, value):
    qHashIteratorHelper(d, value)

def qdump__QHash__iterator(d, value):
    qHashIteratorHelper(d, value)


def qdump__QHostAddress(d, value):
    # QHostAddress in Qt 4.5 (byte offsets)
    #   quint32 a        (0)
    #   Q_IPV6ADDR a6    (4)
    #   protocol         (20)
    #   QString ipString (24)
    #   QString scopeId  (24 + ptrSize)
    #   bool isParsed    (24 + 2 * ptrSize)
    # QHostAddress in Qt 5.0
    #   QString ipString (0)
    #   QString scopeId  (ptrSize)
    #   quint32 a        (2*ptrSize)
    #   Q_IPV6ADDR a6    (2*ptrSize + 4)
    #   protocol         (2*ptrSize + 20)
    #   bool isParsed    (2*ptrSize + 24)

    privAddress = d.extractPointer(value)
    isQt5 = d.qtVersion() >= 0x050000
    sizeofQString = d.ptrSize()
    ipStringAddress = privAddress + (0 if isQt5 else 24)
    isParsedAddress = privAddress + 24 + 2 * sizeofQString
    # value.d.d->ipString
    ipString = d.encodeStringHelper(d.extractPointer(ipStringAddress))
    if d.extractByte(isParsedAddress) and len(ipString) > 0:
        d.putValue(ipString, Hex4EncodedLittleEndian)
    else:
        # value.d.d->protocol:
        #  QAbstractSocket::IPv4Protocol = 0
        #  QAbstractSocket::IPv6Protocol = 1
        protoAddress = privAddress + 20 + (2 * sizeofQString if isQt5 else 0);
        proto = d.extractInt(protoAddress)
        if proto == 1:
            # value.d.d->a6
            a6Offset = 4 + (2 * sizeofQString if isQt5 else 0)
            data = d.readMemory(privAddress + a6Offset, 16)
            address = ':'.join("%x" % int(data[i:i+4], 16) for i in xrange(0, 32, 4))
            scopeId = privAddress + sizeofQString + (0 if isQt5 else 24)
            scopeId = d.encodeStringHelper(d.extractPointer(scopeId))
            d.putValue("%s%%%s" % (address, scopeId), IPv6AddressAndHexScopeId)
        elif proto == 0:
            # value.d.d->a
            a = d.extractInt(privAddress + (2 * sizeofQString if isQt5 else 0))
            a, n4 = divmod(a, 256)
            a, n3 = divmod(a, 256)
            a, n2 = divmod(a, 256)
            a, n1 = divmod(a, 256)
            d.putValue("%d.%d.%d.%d" % (n1, n2, n3, n4));
        else:
            d.putValue("<unspecified>")

    d.putPlainChildren(value["d"]["d"].dereference())


def qdump__QIPv6Address(d, value):
    #warn("IPV6.VALUE: %s" % value)
    #warn("IPV6.ADDR: 0x%x" % d.addressOf(value))
    #warn("IPV6.LOADADDR: 0x%x" % value.GetLoadAddress())
    c = value["c"]
    data = d.readMemory(d.addressOf(c), 16)
    d.putValue(':'.join("%x" % int(data[i:i+4], 16) for i in xrange(0, 32, 4)))
    #d.putValue('xx')
    #d.putValue("0x%x - 0x%x" % (d.addressOf(value), d.addressOf(c)))
    #d.putValue("0x%x - 0x%x" % (value.GetAddress(), c.GetAddress()))
    #d.putValue("0x%x - 0x%x" % (value.GetLoadAddress(), c.GetLoadAddress()))
    d.putPlainChildren(c)

def qform__QList():
    return "Assume Direct Storage,Assume Indirect Storage"

def qdump__QList(d, value):
    base = d.extractPointer(value)
    begin = d.extractInt(base + 8)
    end = d.extractInt(base + 12)
    array = base + 16
    if d.qtVersion() < 0x50000:
        array += d.ptrSize()
    d.check(begin >= 0 and end >= 0 and end <= 1000 * 1000 * 1000)
    size = end - begin
    d.check(size >= 0)
    #d.checkRef(private["ref"])

    innerType = d.templateArgument(value.type, 0)

    d.putItemCount(size)
    d.putNumChild(size)
    if d.isExpanded():
        innerSize = innerType.sizeof
        stepSize = d.ptrSize()
        addr = array + begin * stepSize
        # The exact condition here is:
        #  QTypeInfo<T>::isLarge || QTypeInfo<T>::isStatic
        # but this data is available neither in the compiled binary nor
        # in the frontend.
        # So as first approximation only do the 'isLarge' check:
        format = d.currentItemFormat()
        if format == 1:
            isInternal = True
        elif format == 2:
            isInternal = False
        else:
            isInternal = innerSize <= stepSize and d.isMovableType(innerType)
        if isInternal:
            if innerSize == stepSize:
                p = d.createPointerValue(addr, innerType)
                d.putArrayData(innerType, p, size)
            else:
                with Children(d, size, childType=innerType):
                    for i in d.childRange():
                        p = d.createValue(addr + i * stepSize, innerType)
                        d.putSubItem(i, p)
        else:
            # about 0.5s / 1000 items
            with Children(d, size, maxNumChild=2000, childType=innerType):
                for i in d.childRange():
                    p = d.extractPointer(addr + i * stepSize)
                    x = d.createValue(p, innerType)
                    d.putSubItem(i, x)

def qform__QImage():
    return "Normal,Displayed"

def qdump__QImage(d, value):
    # This relies on current QImage layout:
    # QImageData:
    # - QAtomicInt ref
    # - int width, height, depth, nbytes
    # - padding on 64 bit machines
    # - qreal devicePixelRatio  (+20 + padding)  # Assume qreal == double, Qt 5 only
    # - QVector<QRgb> colortable (+20 + padding + gap)
    # - uchar *data (+20 + padding + gap + ptr)
    # [- uchar **jumptable jumptable with Qt 3 suppor]
    # - enum format (+20 + padding + gap + 2 * ptr)

    ptrSize = d.ptrSize()
    isQt5 = d.qtVersion() >= 0x050000
    offset = (3 if isQt5 else 2) * ptrSize
    base = d.extractPointer(d.addressOf(value) + offset)
    if base == 0:
        d.putValue("(invalid)")
        return
    qt3Support = d.isQt3Support()
    width = d.extractInt(base + 4)
    height = d.extractInt(base + 8)
    nbytes = d.extractInt(base + 16)
    padding = d.ptrSize() - d.intSize()
    pixelRatioSize = 8 if isQt5 else 0
    jumpTableSize = ptrSize if qt3Support else 0
    bits = d.extractPointer(base + 20 + padding + pixelRatioSize + ptrSize)
    iformat = d.extractInt(base + 20 + padding + pixelRatioSize + jumpTableSize + 2 * ptrSize)
    d.putValue("(%dx%d)" % (width, height))
    d.putNumChild(1)
    if d.isExpanded():
        with Children(d):
            d.putIntItem("width", width)
            d.putIntItem("height", height)
            d.putIntItem("nbytes", nbytes)
            d.putIntItem("format", iformat)
            with SubItem(d, "data"):
                d.putValue("0x%x" % bits)
                d.putNumChild(0)
                d.putType("void *")

    format = d.currentItemFormat()
    if format == 1:
        d.putDisplay(StopDisplay)
    elif format == 2:
        # This is critical for performance. Writing to an external
        # file using the following is faster when using GDB.
        #   file = tempfile.mkstemp(prefix="gdbpy_")
        #   filename = file[1].replace("\\", "\\\\")
        #   gdb.execute("dump binary memory %s %s %s" %
        #       (filename, bits, bits + nbytes))
        #   d.putDisplay(DisplayImageFile, " %d %d %d %d %s"
        #       % (width, height, nbytes, iformat, filename))
        d.putField("editformat", DisplayImageData)
        d.put('editvalue="')
        d.put('%08x%08x%08x%08x' % (width, height, nbytes, iformat))
        d.put(d.readMemory(bits, nbytes))
        d.put('",')


def qdump__QLinkedList(d, value):
    dd = d.extractPointer(value)
    ptrSize = d.ptrSize()
    n = d.extractInt(dd + 4 + 2 * ptrSize);
    ref = d.extractInt(dd + 2 * ptrSize);
    d.check(0 <= n and n <= 100*1000*1000)
    d.check(-1 <= ref and ref <= 1000)
    d.putItemCount(n)
    d.putNumChild(n)
    if d.isExpanded():
        innerType = d.templateArgument(value.type, 0)
        with Children(d, n, maxNumChild=1000, childType=innerType):
            pp = d.extractPointer(dd)
            for i in d.childRange():
                d.putSubItem(i, d.createValue(pp + 2 * ptrSize, innerType))
                pp = d.extractPointer(pp)

qqLocalesCount = None

def qdump__QLocale(d, value):
    # Check for uninitialized 'index' variable. Retrieve size of
    # QLocale data array from variable in qlocale.cpp.
    # Default is 368 in Qt 4.8, 438 in Qt 5.0.1, the last one
    # being 'System'.
    #global qqLocalesCount
    #if qqLocalesCount is None:
    #    #try:
    #        qqLocalesCount = int(value(ns + 'locale_data_size'))
    #    #except:
    #        qqLocalesCount = 438
    #try:
    #    index = int(value["p"]["index"])
    #except:
    #    try:
    #        index = int(value["d"]["d"]["m_index"])
    #    except:
    #        index = int(value["d"]["d"]["m_data"]...)
    #d.check(index >= 0)
    #d.check(index <= qqLocalesCount)
    d.putStringValue(d.call(value, "name"))
    d.putNumChild(0)
    return
    # FIXME: Poke back for variants.
    if d.isExpanded():
        ns = d.qtNamespace()
        with Children(d, childType=d.lookupType(ns + "QChar"), childNumChild=0):
            d.putCallItem("country", value, "country")
            d.putCallItem("language", value, "language")
            d.putCallItem("measurementSystem", value, "measurementSystem")
            d.putCallItem("numberOptions", value, "numberOptions")
            d.putCallItem("timeFormat_(short)", value,
                "timeFormat", ns + "QLocale::ShortFormat")
            d.putCallItem("timeFormat_(long)", value,
                "timeFormat", ns + "QLocale::LongFormat")
            d.putCallItem("decimalPoint", value, "decimalPoint")
            d.putCallItem("exponential", value, "exponential")
            d.putCallItem("percent", value, "percent")
            d.putCallItem("zeroDigit", value, "zeroDigit")
            d.putCallItem("groupSeparator", value, "groupSeparator")
            d.putCallItem("negativeSign", value, "negativeSign")
            d.putFields(value)


def qdump__QMapNode(d, value):
    d.putEmptyValue()
    d.putNumChild(2)
    if d.isExpanded():
        with Children(d):
            d.putSubItem("key", value["key"])
            d.putSubItem("value", value["value"])


def qdumpHelper__Qt4_QMap(d, value):
    anon = d.childAt(value, 0)
    d_ptr = anon["d"].dereference()
    e_ptr = anon["e"].dereference()
    n = int(d_ptr["size"])
    d.check(0 <= n and n <= 100*1000*1000)
    d.checkRef(d_ptr["ref"])

    d.putItemCount(n)
    d.putNumChild(n)
    if d.isExpanded():
        if n > 10000:
            n = 10000

        keyType = d.templateArgument(value.type, 0)
        valueType = d.templateArgument(value.type, 1)

        it = e_ptr["forward"].dereference()

        # QMapPayloadNode is QMapNode except for the 'forward' member, so
        # its size is most likely the offset of the 'forward' member therein.
        # Or possibly 2 * sizeof(void *)
        # Note: Keeping the spacing in the type lookup
        # below is important for LLDB.
        needle = str(value.type).replace("QMap", "QMapNode", 1)
        needle = d.qtNamespace() + "QMapNode<%s,%s>" % (keyType, valueType)
        nodeType = d.lookupType(needle)
        nodePointerType = nodeType.pointer()
        # symbols reports payload size at wrong size 24
        if d.isArmArchitecture() and d.isQnxTarget() and str(valueType) == 'QVariant':
            payloadSize = 28
        else:
            payloadSize = nodeType.sizeof - 2 * nodePointerType.sizeof

        with PairedChildren(d, n, useKeyAndValue=True,
                keyType=keyType, valueType=valueType, pairType=nodeType):
            for i in xrange(n):
                base = it.cast(d.charPtrType()) - payloadSize
                node = base.cast(nodePointerType).dereference()
                with SubItem(d, i):
                    #d.putField("iname", d.currentIName)
                    d.putPair(node, i)
                it = it.dereference()["forward"].dereference()


def qdumpHelper__Qt5_QMap(d, value):
    d_ptr = value["d"].dereference()
    n = int(d_ptr["size"])
    d.check(0 <= n and n <= 100*1000*1000)
    d.checkRef(d_ptr["ref"])

    d.putItemCount(n)
    d.putNumChild(n)
    if d.isExpanded():
        if n > 10000:
            n = 10000

        keyType = d.templateArgument(value.type, 0)
        valueType = d.templateArgument(value.type, 1)
        # Note: Keeping the spacing in the type lookup
        # below is important for LLDB.
        needle = str(d_ptr.type).replace("QMapData", "QMapNode", 1)
        nodeType = d.lookupType(needle)

        def helper(d, node, nodeType, i):
            left = node["left"]
            if not d.isNull(left):
                i = helper(d, left.dereference(), nodeType, i)
                if i >= n:
                    return i

            nodex = node.cast(nodeType)
            with SubItem(d, i):
                d.putPair(nodex, i)

            i += 1
            if i >= n:
                return i

            right = node["right"]
            if not d.isNull(right):
                i = helper(d, right.dereference(), nodeType, i)

            return i

        with PairedChildren(d, n, useKeyAndValue=True,
                keyType=keyType, valueType=valueType, pairType=nodeType):
            node = d_ptr["header"]
            helper(d, node, nodeType, 0)


def qform__QMap():
    return mapForms()

def qdump__QMap(d, value):
    if d.qtVersion() < 0x50000:
        qdumpHelper__Qt4_QMap(d, value)
    else:
        qdumpHelper__Qt5_QMap(d, value)

def qform__QMultiMap():
    return mapForms()

def qdump__QMultiMap(d, value):
    qdump__QMap(d, value)


def extractCString(table, offset):
    result = ""
    while True:
        d = table[offset]
        if d == 0:
            break
        result += "%c" % d
        offset += 1
    return result

def qdump__QMetaObjectPrivate(d, value):
    d.putEmptyValue()
    d.putNumChild(1)
    if d.isExpanded():
        with Children(d):
            # int revision;
            # int className;
            # int classInfoCount, classInfoData;
            # int methodCount, methodData;
            # int propertyCount, propertyData;
            # int enumeratorCount, enumeratorData;
            # int constructorCount, constructorData; //since revision 2
            # int flags; //since revision 3
            # int signalCount; //since revision 4
            d.putIntItem("revision", value["revision"])
            d.putIntItem("methodCount", value["methodCount"])
            d.putIntItem("propertyCount", value["propertyCount"])
            d.putIntItem("enumeratorCount", value["enumeratorCount"])
            d.putIntItem("constructorCount", value["constructorCount"])
            d.putIntItem("flags", value["flags"])
            d.putIntItem("signalCount", value["signalCount"])



def qdump__QMetaObject(d, value):
    d.putEmptyValue()
    d.putNumChild(1)
    if d.isExpanded():
        with Children(d):
            dd = value["d"]
            d.putSubItem("d", dd)
            data = d.extractPointer(dd["data"])

            propertyNames = d.staticQObjectPropertyNames(value)
            propertyIndex = 0
            for propertyName in propertyNames:
                with SubItem(d, "property_%s" % propertyIndex):
                    d.putValue(propertyName)
                    propertyIndex += 1

           #byteArrayDataType = d.lookupType(d.qtNamespace() + "QByteArrayData")
           #byteArrayDataSize = byteArrayDataType.sizeof
           #sd = d.extractPointer(dd["stringdata"])
           #stringdata, size, alloc = d.byteArrayDataHelper(sd)

           #propertyCount = d.extractInt(data + 24)
           #propertyData = d.extractInt(data + 28)

           ## This is the 'data' member in the qt_meta_stringdata_qobject__*_t struct
           #d.putIntItem("_byteArrayDataSize", byteArrayDataSize)
           #d.putAddressItem("_data", data)
           #d.putAddressItem("_sd_", stringdata)
           #with SubItem(d, "_sd"):
           #    d.putValue(d.readMemory(stringdata, size), Hex2EncodedLatin1)
           #with SubItem(d, "_cn"):
           #    d.putValue(d.readMemory(stringdata + d.extractInt(data + 4), size), Hex2EncodedLatin1)

           #for i in range(propertyCount):
           #    with SubItem(d, "property_%s" % i):
           #        x = data + (propertyData + 3 * i) * 4
           #        literal = sd + d.extractInt(x) * byteArrayDataSize
           #        ldata, lsize, lalloc = d.byteArrayDataHelper(literal)
           #        d.putValue(d.readMemory(ldata, lsize), Hex2EncodedLatin1)

           #        d.putNumChild(1)
           #        if d.isExpanded():
           #            with Children(d):
           #                if d.isExpanded():
           #                    d.putAddressItem("_literal", literal)
           #                    d.putIntItem("__data", ldata)
           #                    d.putIntItem("__size", lsize)
           #                    d.putIntItem("__alloc", lalloc)
           #                    d.putIntItem("name", d.extractInt(x))
           #                    d.putIntItem("type", d.extractInt(x + 4))
           #                    d.putIntItem("flags", d.extractInt(x + 8))

            methodCount = d.extractInt(data + 16)
            methodData = d.extractInt(data + 20)
            for i in range(methodCount):
                with SubItem(d, "method_%s" % i):
                    x = data + (methodData + 5 * i) * 4
                    #d.putEmptyValue()
                    d.putValue(d.readCString(stringdata + d.extractInt(x)))
                    d.putNumChild(1)
                    if d.isExpanded():
                        with Children(d):
                            if d.isExpanded():
                                d.putIntItem("name", d.extractInt(x))
                                d.putIntItem("argc", d.extractInt(x + 4))
                                d.putIntItem("argv", d.extractInt(x + 8))
                                d.putIntItem("type", d.extractInt(x + 12))
                                d.putIntItem("flags", d.extractInt(x + 16))

            d.putSubItem("stringData", dd["stringdata"])
            d.putIntItem("revision", d.extractInt(data))
            d.putIntItem("className", d.extractInt(data + 4))
            d.putIntItem("classInfoCount", d.extractInt(data + 8))
            d.putIntItem("className", d.extractInt(data + 12))
            d.putIntItem("methodCount", d.extractInt(data + 16))
            d.putIntItem("methodData", d.extractInt(data + 20))
            d.putIntItem("propertyCount", d.extractInt(data + 24))
            d.putIntItem("propertyData", d.extractInt(data + 28))
            d.putIntItem("enumeratorCount", d.extractInt(data + 32))
            d.putIntItem("enumeratorData", d.extractInt(data + 36))
            d.putIntItem("constructorCount", d.extractInt(data + 40))
            d.putIntItem("constructorData", d.extractInt(data + 44))
            d.putIntItem("flags", d.extractInt(data + 48))
            d.putIntItem("signalCount", d.extractInt(data + 52))

def _qdump__QObject(d, value):
    d.putQObjectNameValue(value)
    ns = d.qtNamespace()

    try:
        privateTypeName = ns + "QObjectPrivate"
        privateType = d.lookupType(privateTypeName)
        staticMetaObject = value["staticMetaObject"]
    except:
        d.putPlainChildren(value)
        return
    #warn("SMO: %s " % staticMetaObject)
    #warn("SMO DATA: %s " % staticMetaObject["d"]["stringdata"])
    superData = staticMetaObject["d"]["superdata"]
    #warn("SUPERDATA: %s" % superData)
    #while not d.isNull(superData):
    #    superData = superData.dereference()["d"]["superdata"]
    #    warn("SUPERDATA: %s" % superData)

    if privateType is None:
        #d.putValue(d.cleanAddress(d.pointerValue(value))
        d.putPlainChildren(value)
        return

    #warn("OBJECTNAME: %s " % objectName)
    dd = value["d_ptr"]["d"]
    d_ptr = dd.cast(privateType.pointer()).dereference()
    #warn("D_PTR: %s " % d_ptr)
    mo = d_ptr["metaObject"]
    if d.isNull(mo):
        mo = staticMetaObject
    #warn("MO: %s " % mo)
    #warn("MO.D: %s " % mo["d"])
    metaData = mo["d"]["data"]
    metaStringData = mo["d"]["stringdata"]
    # This is char * in Qt 4 and ByteArrayData * in Qt 5.
    # Force it to the char * data in the Qt 5 case.
    try:
        offset = metaStringData["offset"]
        metaStringData = metaStringData.cast(d.charPtrType()) + int(offset)
    except:
        pass

    #extradata = mo["d"]["extradata"]   # Capitalization!
    #warn("METADATA: %s " % metaData)
    #warn("STRINGDATA: %s " % metaStringData)
    #warn("TYPE: %s " % value.type)
    #warn("INAME: %s " % d.currentIName)
    d.putEmptyValue()
    #QSignalMapper::staticMetaObject
    #d.checkRef(d_ptr["ref"])
    d.putNumChild(4)
    if d.isExpanded():
      with Children(d):
        d.putQObjectGuts(value)

        # Local data.
        if privateTypeName != ns + "QObjectPrivate":
            if not privateType is None:
              with SubItem(d, "data"):
                d.putEmptyValue()
                d.putNoType()
                d.putPlainChildren(d_ptr, False)

        d.putFields(value)
        # Parent and children.
        if stripClassTag(str(value.type)) == ns + "QObject":
            d.putSubItem("parent", d_ptr["parent"])
            d.putSubItem("children", d_ptr["children"])

        # Metaobject.
        d.putSubItem("metaobject", mo)

        # Dynamic Properties.
        with SubItem(d, "dynamics"):
            # Prolog
            extraData = d_ptr["extraData"]   # Capitalization!
            if d.isNull(extraData):
                dynamicPropertyCount = 0
            else:
                extraDataType = d.lookupType(
                    ns + "QObjectPrivate::ExtraData").pointer()
                extraData = extraData.cast(extraDataType)
                ed = extraData.dereference()
                names = ed["propertyNames"]
                values = ed["propertyValues"]
                #userData = ed["userData"]
                namesBegin = names["d"]["begin"]
                namesEnd = names["d"]["end"]
                namesArray = names["d"]["array"]
                dynamicPropertyCount = namesEnd - namesBegin

            d.putNoType()
            d.putItemCount(dynamicPropertyCount)
            d.putNumChild(dynamicPropertyCount)

            if d.isExpanded() and d.isGdb:
                import gdb
                # FIXME: Make this global. Don't leak.
                variant = "'%sQVariant'" % ns
                # Avoid malloc symbol clash with QVector
                gdb.execute("set $d = (%s*)calloc(sizeof(%s), 1)"
                    % (variant, variant))
                gdb.execute("set $d.d.is_shared = 0")

                with Children(d):
                    dummyType = d.voidPtrType().pointer()
                    namesType = d.lookupType(ns + "QByteArray")
                    valuesBegin = values["d"]["begin"]
                    valuesEnd = values["d"]["end"]
                    valuesArray = values["d"]["array"]
                    valuesType = d.lookupType(ns + "QVariant")
                    p = namesArray.cast(dummyType) + namesBegin
                    q = valuesArray.cast(dummyType) + valuesBegin
                    for i in xrange(dynamicPropertyCount):
                        with SubItem(d, i):
                            pp = p.cast(namesType.pointer()).dereference();
                            d.putField("key", d.encodeByteArray(pp))
                            d.putField("keyencoded", Hex2EncodedLatin1)
                            qq = q.cast(valuesType.pointer().pointer())
                            qq = qq.dereference();
                            d.putField("addr", d.cleanAddress(qq))
                            d.putField("exp", "*(%s*)%s"
                                 % (variant, d.cleanAddress(qq)))
                            t = qdump__QVariant(d, qq)
                            # Override the "QVariant (foo)" output.
                            d.putBetterType(t)
                        p += 1
                        q += 1


        # Connections.
        with SubItem(d, "connections"):
            d.putNoType()
            connections = d_ptr["connectionLists"]
            connectionListCount = 0
            if not d.isNull(connections):
                connectionListCount = connections["d"]["size"]
            d.putItemCount(connectionListCount, 0)
            d.putNumChild(connectionListCount)
            if d.isExpanded():
                pp = 0
                with Children(d):
                    vectorType = d.fieldAt(connections.type.target(), 0).type
                    innerType = d.templateArgument(vectorType, 0)
                    # Should check:  innerType == ns::QObjectPrivate::ConnectionList
                    p = gdb.Value(connections["p"]["array"]).cast(innerType.pointer())
                    for i in xrange(connectionListCount):
                        first = p.dereference()["first"]
                        while not d.isNull(first):
                            with SubItem(d, pp):
                                connection = first.dereference()
                                d.putItem(connection)
                                d.putValue(connection["callFunction"])
                            first = first["nextConnectionList"]
                            # We need to enforce some upper limit.
                            pp += 1
                            if pp > 1000:
                                break
                        p += 1
                if pp < 1000:
                    d.putItemCount(pp)


        # Active connection.
        with SubItem(d, "currentSender"):
            d.putNoType()
            sender = d_ptr["currentSender"]
            d.putPointerValue(sender)
            if d.isNull(sender):
                d.putNumChild(0)
            else:
                d.putNumChild(1)
                if d.isExpanded():
                    with Children(d):
                        # Sending object
                        d.putSubItem("object", sender["sender"])
                        # Signal in sending object
                        with SubItem(d, "signal"):
                            d.putValue(sender["signal"])
                            d.putNoType()
                            d.putNumChild(0)

# QObject

#   static const uint qt_meta_data_QObject[] = {

#   int revision;
#   int className;
#   int classInfoCount, classInfoData;
#   int methodCount, methodData;
#   int propertyCount, propertyData;
#   int enumeratorCount, enumeratorData;
#   int constructorCount, constructorData; //since revision 2
#   int flags; //since revision 3
#   int signalCount; //since revision 4

#    // content:
#          4,       // revision
#          0,       // classname
#          0,    0, // classinfo
#          4,   14, // methods
#          1,   34, // properties
#          0,    0, // enums/sets
#          2,   37, // constructors
#          0,       // flags
#          2,       // signalCount

#  /* 14 */

#    // signals: signature, parameters, type, tag, flags
#          9,    8,    8,    8, 0x05,
#         29,    8,    8,    8, 0x25,

#  /* 24 */
#    // slots: signature, parameters, type, tag, flags
#         41,    8,    8,    8, 0x0a,
#         55,    8,    8,    8, 0x08,

#  /* 34 */
#    // properties: name, type, flags
#         90,   82, 0x0a095103,

#  /* 37 */
#    // constructors: signature, parameters, type, tag, flags
#        108,  101,    8,    8, 0x0e,
#        126,    8,    8,    8, 0x2e,

#          0        // eod
#   };

#   static const char qt_meta_stringdata_QObject[] = {
#       "QObject\0\0destroyed(QObject*)\0destroyed()\0"
#       "deleteLater()\0_q_reregisterTimers(void*)\0"
#       "QString\0objectName\0parent\0QObject(QObject*)\0"
#       "QObject()\0"
#   };


# QSignalMapper

#   static const uint qt_meta_data_QSignalMapper[] = {

#    // content:
#          4,       // revision
#          0,       // classname
#          0,    0, // classinfo
#          7,   14, // methods
#          0,    0, // properties
#          0,    0, // enums/sets
#          0,    0, // constructors
#          0,       // flags
#          4,       // signalCount

#    // signals: signature, parameters, type, tag, flags
#         15,   14,   14,   14, 0x05,
#         27,   14,   14,   14, 0x05,
#         43,   14,   14,   14, 0x05,
#         60,   14,   14,   14, 0x05,

#    // slots: signature, parameters, type, tag, flags
#         77,   14,   14,   14, 0x0a,
#         90,   83,   14,   14, 0x0a,
#        104,   14,   14,   14, 0x08,

#          0        // eod
#   };

#   static const char qt_meta_stringdata_QSignalMapper[] = {
#       "QSignalMapper\0\0mapped(int)\0mapped(QString)\0"
#       "mapped(QWidget*)\0mapped(QObject*)\0"
#       "map()\0sender\0map(QObject*)\0"
#       "_q_senderDestroyed()\0"
#   };

#   const QMetaObject QSignalMapper::staticMetaObject = {
#       { &QObject::staticMetaObject, qt_meta_stringdata_QSignalMapper,
#         qt_meta_data_QSignalMapper, 0 }
#   };



# // Meta enumeration helpers
# static inline void dumpMetaEnumType(QDumper &d, const QMetaEnum &me)
# {
#     QByteArray type = me.scope()
#     if !type.isEmpty())
#         type += "::"
#     type += me.name()
#     d.putField("type", type.constData())
# }
#
# static inline void dumpMetaEnumValue(QDumper &d, const QMetaProperty &mop,
#                                      int value)
# {
#
#     const QMetaEnum me = mop.enumerator()
#     dumpMetaEnumType(d, me)
#     if const char *enumValue = me.valueToKey(value)) {
#         d.putValue(enumValue)
#     } else {
#         d.putValue(value)
#     }
#     d.putField("numchild", 0)
# }
#
# static inline void dumpMetaFlagValue(QDumper &d, const QMetaProperty &mop,
#                                      int value)
# {
#     const QMetaEnum me = mop.enumerator()
#     dumpMetaEnumType(d, me)
#     const QByteArray flagsValue = me.valueToKeys(value)
#     if flagsValue.isEmpty():
#         d.putValue(value)
#     else:
#         d.putValue(flagsValue.constData())
#     d.putNumChild(0)
# }

def qdump__QPixmap(d, value):
    offset = (3 if d.qtVersion() >= 0x050000 else 2) * d.ptrSize()
    base = d.extractPointer(d.addressOf(value) + offset)
    if base == 0:
        d.putValue("(invalid)")
    else:
        width = d.extractInt(base + d.ptrSize())
        height = d.extractInt(base + d.ptrSize() + 4)
        d.putValue("(%dx%d)" % (width, height))
    d.putNumChild(0)


def qdump__QPoint(d, value):
    x = int(value["xp"])
    y = int(value["yp"])
    d.putValue("(%s, %s)" % (x, y))
    d.putPlainChildren(value)


def qdump__QPointF(d, value):
    x = float(value["xp"])
    y = float(value["yp"])
    d.putValue("(%s, %s)" % (x, y))
    d.putPlainChildren(value)


def qdump__QRect(d, value):
    def pp(l):
        if l >= 0: return "+%s" % l
        return l
    x1 = int(value["x1"])
    y1 = int(value["y1"])
    x2 = int(value["x2"])
    y2 = int(value["y2"])
    w = x2 - x1 + 1
    h = y2 - y1 + 1
    d.putValue("%sx%s%s%s" % (w, h, pp(x1), pp(y1)))
    d.putPlainChildren(value)


def qdump__QRectF(d, value):
    def pp(l):
        if l >= 0: return "+%s" % l
        return l
    x = float(value["xp"])
    y = float(value["yp"])
    w = float(value["w"])
    h = float(value["h"])
    d.putValue("%sx%s%s%s" % (w, h, pp(x), pp(y)))
    d.putPlainChildren(value)


def qdump__QRegExp(d, value):
    # value.priv.engineKey.pattern
    privAddress = d.extractPointer(value)
    engineKeyAddress = privAddress + d.ptrSize()
    patternAddress = engineKeyAddress
    d.putStringValueByAddress(patternAddress)
    d.putNumChild(1)
    if d.isExpanded():
        with Children(d):
            # QRegExpPrivate:
            # - QRegExpEngine *eng               (+0)
            # - QRegExpEngineKey:                (+1ptr)
            #   - QString pattern;               (+1ptr)
            #   - QRegExp::PatternSyntax patternSyntax;  (+2ptr)
            #   - Qt::CaseSensitivity cs;        (+2ptr +1enum +pad?)
            # - bool minimal                     (+2ptr +2enum +2pad?)
            # - QString t                        (+2ptr +2enum +1bool +3pad?)
            # - QStringList captures             (+3ptr +2enum +1bool +3pad?)
            # FIXME: Remove need to call. Needed to warm up cache.
            d.call(value, "capturedTexts") # create cache
            ns = d.qtNamespace()
            with SubItem(d, "syntax"):
                # value["priv"]["engineKey"["capturedCache"]
                address = engineKeyAddress + d.ptrSize()
                typ = d.lookupType(ns + "QRegExp::PatternSyntax")
                d.putItem(d.createValue(address, typ))
            with SubItem(d, "captures"):
                # value["priv"]["capturedCache"]
                address = privAddress + 3 * d.ptrSize() + 12
                typ = d.lookupType(ns + "QStringList")
                d.putItem(d.createValue(address, typ))


def qdump__QRegion(d, value):
    p = value["d"].dereference()["qt_rgn"]
    if d.isNull(p):
        d.putValue("<empty>")
        d.putNumChild(0)
    else:
        # struct QRegionPrivate:
        # int numRects;
        # QVector<QRect> rects;
        # QRect extents;
        # QRect innerRect;
        # int innerArea;
        pp = d.extractPointer(p)
        n = d.extractInt(pp)
        d.putItemCount(n)
        d.putNumChild(n)
        if d.isExpanded():
            with Children(d):
                v = d.ptrSize()
                ns = d.qtNamespace()
                rectType = d.lookupType(ns + "QRect")
                d.putIntItem("numRects", n)
                d.putSubItem("extents", d.createValue(pp + 2 * v, rectType))
                d.putSubItem("innerRect", d.createValue(pp + 2 * v + rectType.sizeof, rectType))
                d.putIntItem("innerArea", d.extractInt(pp + 2 * v + 2 * rectType.sizeof))
                # FIXME
                try:
                    # Can fail if QVector<QRect> debuginfo is missing.
                    vectType = d.lookupType("%sQVector<%sQRect>" % (ns, ns))
                    d.putSubItem("rects", d.createValue(pp + v, vectType))
                except:
                    with SubItem(d, "rects"):
                        d.putItemCount(n)
                        d.putType("%sQVector<%sQRect>" % (ns, ns))
                        d.putNumChild(0)


def qdump__QScopedPointer(d, value):
    d.putBetterType(d.currentType)
    d.putItem(value["d"])


def qdump__QSet(d, value):

    def hashDataFirstNode(dPtr, numBuckets):
        ePtr = dPtr.cast(nodeTypePtr)
        bucket = dPtr["buckets"]
        for n in xrange(numBuckets - 1, -1, -1):
            n = n - 1
            if n < 0:
                break
            if d.pointerValue(bucket.dereference()) != d.pointerValue(ePtr):
                return bucket.dereference()
            bucket = bucket + 1
        return ePtr

    def hashDataNextNode(nodePtr, numBuckets):
        nextPtr = nodePtr.dereference()["next"]
        if d.pointerValue(nextPtr.dereference()["next"]):
            return nextPtr
        dPtr = nodePtr.cast(hashDataType.pointer()).dereference()
        start = (int(nodePtr.dereference()["h"]) % numBuckets) + 1
        bucket = dPtr.dereference()["buckets"] + start
        for n in xrange(numBuckets - start):
            if d.pointerValue(bucket.dereference()) != d.pointerValue(nextPtr):
                return bucket.dereference()
            bucket += 1
        return nodePtr

    anon = d.childAt(value, 0)
    if d.isLldb: # Skip the inheritance level.
        anon = d.childAt(anon, 0)
    d_ptr = anon["d"]
    e_ptr = anon["e"]
    size = int(d_ptr.dereference()["size"])

    d.check(0 <= size and size <= 100 * 1000 * 1000)
    d.checkRef(d_ptr["ref"])

    d.putItemCount(size)
    d.putNumChild(size)
    if d.isExpanded():
        hashDataType = d_ptr.type
        nodeTypePtr = d_ptr.dereference()["fakeNext"].type
        numBuckets = int(d_ptr.dereference()["numBuckets"])
        innerType = e_ptr.dereference().type
        with Children(d, size, maxNumChild=1000, childType=innerType):
            for i in d.childRange():
                if i == 0:
                    node = hashDataFirstNode(d_ptr, numBuckets)
                else:
                    node = hashDataNextNode(node, numBuckets)
                it = node.dereference().cast(innerType)
                with SubItem(d, i):
                    key = it["key"]
                    if not key:
                        # LLDB can't access directly since it's in anonymous union
                        # for Qt4 optimized int keytype
                        key = it[1]["key"]
                    d.putItem(key)


def qdump__QSharedData(d, value):
    d.putValue("ref: %s" % value["ref"]["_q_value"])
    d.putNumChild(0)


def qdump__QSharedDataPointer(d, value):
    d_ptr = value["d"]
    if d.isNull(d_ptr):
        d.putValue("(null)")
        d.putNumChild(0)
    else:
        # This replaces the pointer by the pointee, making the
        # pointer transparent.
        try:
            innerType = d.templateArgument(value.type, 0)
        except:
            d.putValue(d_ptr)
            d.putPlainChildren(value)
            return
        d.putBetterType(d.currentType)
        d.putItem(d_ptr.cast(innerType.pointer()).dereference())


def qdump__QSharedPointer(d, value):
    qdump__QWeakPointer(d, value)


def qdump__QSize(d, value):
    w = int(value["wd"])
    h = int(value["ht"])
    d.putValue("(%s, %s)" % (w, h))
    d.putPlainChildren(value)

def qdump__QSizeF(d, value):
    w = float(value["wd"])
    h = float(value["ht"])
    d.putValue("(%s, %s)" % (w, h))
    d.putPlainChildren(value)


def qform__QStack():
    return arrayForms()

def qdump__QStack(d, value):
    qdump__QVector(d, value)


def qdump__QStandardItem(d, value):
    d.putBetterType(d.currentType)
    try:
        d.putItem(value["d_ptr"])
    except:
        d.putPlainChildren(value)


def qedit__QString(d, value, data):
    d.call(value, "resize", str(len(data)))
    (base, size, alloc) = d.stringData(value)
    d.setValues(base, "short", [ord(c) for c in data])

def qform__QString():
    return "Inline,Separate Window"

def qdump__QString(d, value):
    d.putStringValue(value)
    d.putNumChild(0)
    format = d.currentItemFormat()
    if format == 1:
        d.putDisplay(StopDisplay)
    elif format == 2:
        d.putField("editformat", DisplayUtf16String)
        d.putField("editvalue", d.encodeString(value))


def qdump__QStringRef(d, value):
    if d.isNull(value["m_string"]):
        d.putValue("(null)");
        d.putNumChild(0)
        return
    s = value["m_string"].dereference()
    data, size, alloc = d.stringData(s)
    data += 2 * int(value["m_position"])
    size = int(value["m_size"])
    s = d.readMemory(data, 2 * size)
    d.putValue(s, Hex4EncodedLittleEndian)
    d.putPlainChildren(value)


def qdump__QStringList(d, value):
    listType = d.directBaseClass(value.type)
    qdump__QList(d, value.cast(listType))
    d.putBetterType(value.type)


def qdump__QTemporaryFile(d, value):
    qdump__QFile(d, value)


def qdump__QTextCodec(d, value):
    name = d.call(value, "name")
    d.putValue(d.encodeByteArray(d, name), 6)
    d.putNumChild(2)
    if d.isExpanded():
        with Children(d):
            d.putCallItem("name", value, "name")
            d.putCallItem("mibEnum", value, "mibEnum")
            d.putFields(value)


def qdump__QTextCursor(d, value):
    privAddress = d.extractPointer(value)
    if privAddress == 0:
        d.putValue("(invalid)")
        d.putNumChild(0)
    else:
        positionAddress = privAddress + 2 * d.ptrSize() + 8
        d.putValue(d.extractInt(positionAddress))
        d.putNumChild(1)
    if d.isExpanded():
        with Children(d):
            positionAddress = privAddress + 2 * d.ptrSize() + 8
            d.putIntItem("position", d.extractInt(positionAddress))
            d.putIntItem("anchor", d.extractInt(positionAddress + d.intSize()))
            d.putCallItem("selected", value, "selectedText")
            d.putFields(value)


def qdump__QTextDocument(d, value):
    d.putEmptyValue()
    d.putNumChild(1)
    if d.isExpanded():
        with Children(d):
            d.putCallItem("blockCount", value, "blockCount")
            d.putCallItem("characterCount", value, "characterCount")
            d.putCallItem("lineCount", value, "lineCount")
            d.putCallItem("revision", value, "revision")
            d.putCallItem("toPlainText", value, "toPlainText")
            d.putFields(value)


def qform__QUrl():
    return "Inline,Separate Window"

def qdump__QUrl(d, value):
    if d.qtVersion() < 0x050000:
        privAddress = d.extractPointer(value)
        if not privAddress:
            # d == 0 if QUrl was constructed with default constructor
            d.putValue("<invalid>")
            return
        encodedOriginalAddress = privAddress + 8 * d.ptrSize()
        d.putValue(d.encodeByteArrayHelper(d.extractPointer(encodedOriginalAddress)), Hex2EncodedLatin1)
        d.putNumChild(8)
        if d.isExpanded():
            stringType = d.lookupType(d.qtNamespace() + "QString")
            baType = d.lookupType(d.qtNamespace() + "QByteArray")
            with Children(d):
                # Qt 4 only decodes the original string if some detail is requested
                d.putCallItem("scheme", value, "scheme")
                d.putCallItem("userName", value, "userName")
                d.putCallItem("password", value, "password")
                d.putCallItem("host", value, "host")
                d.putCallItem("path", value, "path")
                d.putCallItem("query", value, "encodedQuery")
                d.putCallItem("fragment", value, "fragment")
                d.putCallItem("port", value, "port")
                d.putFields(value)
    else:
        # QUrlPrivate:
        # - QAtomicInt ref;
        # - int port;
        # - QString scheme;
        # - QString userName;
        # - QString password;
        # - QString host;
        # - QString path;
        # - QString query;
        # - QString fragment;
        privAddress = d.extractPointer(value)
        if not privAddress:
            # d == 0 if QUrl was constructed with default constructor
            d.putValue("<invalid>")
            return
        schemeAddr = privAddress + 2 * d.intSize()
        scheme = d.encodeStringHelper(d.extractPointer(schemeAddr))
        userName = d.encodeStringHelper(d.extractPointer(schemeAddr + 1 * d.ptrSize()))
        password = d.encodeStringHelper(d.extractPointer(schemeAddr + 2 * d.ptrSize()))
        host = d.encodeStringHelper(d.extractPointer(schemeAddr + 3 * d.ptrSize()))
        path = d.encodeStringHelper(d.extractPointer(schemeAddr + 4 * d.ptrSize()))
        query = d.encodeStringHelper(d.extractPointer(schemeAddr + 5 * d.ptrSize()))
        fragment = d.encodeStringHelper(d.extractPointer(schemeAddr + 6 * d.ptrSize()))
        port = d.extractInt(d.extractPointer(value) + d.intSize())

        url = scheme
        url += "3a002f002f00"
        if len(userName):
            url += userName
            url += "4000"
        url += host
        if port >= 0:
            url += "3a00"
            url += ''.join(["%02x00" % ord(c) for c in str(port)])
        url += path
        d.putValue(url, Hex4EncodedLittleEndian)

        format = d.currentItemFormat()
        if format == 1:
            d.putDisplay(StopDisplay)
        elif format == 2:
            d.putField("editformat", DisplayUtf16String)
            d.putField("editvalue", url)

        d.putNumChild(8)
        if d.isExpanded():
            stringType = d.lookupType(d.qtNamespace() + "QString")
            with Children(d):
                d.putIntItem("port", port)
                d.putGenericItem("scheme", stringType, scheme, Hex4EncodedLittleEndian)
                d.putGenericItem("userName", stringType, userName, Hex4EncodedLittleEndian)
                d.putGenericItem("password", stringType, password, Hex4EncodedLittleEndian)
                d.putGenericItem("host", stringType, host, Hex4EncodedLittleEndian)
                d.putGenericItem("path", stringType, path, Hex4EncodedLittleEndian)
                d.putGenericItem("query", stringType, query, Hex4EncodedLittleEndian)
                d.putGenericItem("fragment", stringType, fragment, Hex4EncodedLittleEndian)

def qdumpHelper_QVariant_0(d, blob):
    # QVariant::Invalid
    d.putBetterType("%sQVariant (invalid)" % d.qtNamespace())
    d.putValue("(invalid)")

def qdumpHelper_QVariant_1(d, blob):
    # QVariant::Bool
    d.putBetterType("%sQVariant (bool)" % d.qtNamespace())
    d.putValue("true" if blob.extractByte() else "false")

def qdumpHelper_QVariant_2(d, blob):
    # QVariant::Int
    d.putBetterType("%sQVariant (int)" % d.qtNamespace())
    d.putValue("%s" % blob.extractInt())

def qdumpHelper_QVariant_3(d, blob):
    # uint
    d.putBetterType("%sQVariant (uint)" % d.qtNamespace())
    d.putValue(blob.extractUInt())

def qdumpHelper_QVariant_4(d, blob):
    # qlonglong
    d.putBetterType("%sQVariant (qlonglong)" % d.qtNamespace())
    d.putValue(blob.extractInt64())

def qdumpHelper_QVariant_5(d, blob):
    # qulonglong
    d.putBetterType("%sQVariant (qulonglong)" % d.qtNamespace())
    d.putValue(blob.extractUInt64())

def qdumpHelper_QVariant_6(d, blob):
    # QVariant::Double
    d.putBetterType("%sQVariant (double)" % d.qtNamespace())
    d.putValue(blob.extractDouble())

qdumpHelper_QVariants_A = [
    qdumpHelper_QVariant_0,
    qdumpHelper_QVariant_1,
    qdumpHelper_QVariant_2,
    qdumpHelper_QVariant_3,
    qdumpHelper_QVariant_4,
    qdumpHelper_QVariant_5,
    qdumpHelper_QVariant_6
]


qdumpHelper_QVariants_B = [
    "QChar",       # 7
    "QVariantMap", # 8
    "QVariantList",# 9
    "QString",     # 10
    "QStringList", # 11
    "QByteArray",  # 12
    "QBitArray",   # 13
    "QDate",       # 14
    "QTime",       # 15
    "QDateTime",   # 16
    "QUrl",        # 17
    "QLocale",     # 18
    "QRect",       # 19
    "QRectF",      # 20
    "QSize",       # 21
    "QSizeF",      # 22
    "QLine",       # 23
    "QLineF",      # 24
    "QPoint",      # 25
    "QPointF",     # 26
    "QRegExp",     # 27
    "QVariantHash",# 28
]

def qdumpHelper_QVariant_31(d, blob):
    # QVariant::VoidStar
    d.putBetterType("%sQVariant (void *)" % d.qtNamespace())
    d.putValue("0x%x" % d.extractPointer(blob))

def qdumpHelper_QVariant_32(d, blob):
    # QVariant::Long
    d.putBetterType("%sQVariant (long)" % d.qtNamespace())
    d.putValue("%s" % blob.extractLong())

def qdumpHelper_QVariant_33(d, blob):
    # QVariant::Short
    d.putBetterType("%sQVariant (short)" % d.qtNamespace())
    d.putValue("%s" % blob.extractShort())

def qdumpHelper_QVariant_34(d, blob):
    # QVariant::Char
    d.putBetterType("%sQVariant (char)" % d.qtNamespace())
    d.putValue("%s" % blob.extractByte())

def qdumpHelper_QVariant_35(d, blob):
    # QVariant::ULong
    d.putBetterType("%sQVariant (unsigned long)" % d.qtNamespace())
    d.putValue("%s" % blob.extractULong())

def qdumpHelper_QVariant_36(d, blob):
    # QVariant::UShort
    d.putBetterType("%sQVariant (unsigned short)" % d.qtNamespace())
    d.putValue("%s" % blob.extractUShort())

def qdumpHelper_QVariant_37(d, blob):
    # QVariant::UChar
    d.putBetterType("%sQVariant (unsigned char)" % d.qtNamespace())
    d.putValue("%s" % blob.extractByte())

def qdumpHelper_QVariant_38(d, blob):
    # QVariant::Float
    d.putBetterType("%sQVariant (float)" % d.qtNamespace())
    d.putValue("%s" % blob.extractFloat())

qdumpHelper_QVariants_D = [
    qdumpHelper_QVariant_31,
    qdumpHelper_QVariant_32,
    qdumpHelper_QVariant_33,
    qdumpHelper_QVariant_34,
    qdumpHelper_QVariant_35,
    qdumpHelper_QVariant_36,
    qdumpHelper_QVariant_37,
    qdumpHelper_QVariant_38
]

qdumpHelper_QVariants_E = [
    "QFont",       # 64
    "QPixmap",     # 65
    "QBrush",      # 66
    "QColor",      # 67
    "QPalette",    # 68
    "QIcon",       # 69
    "QImage",      # 70
    "QPolygon",    # 71
    "QRegion",     # 72
    "QBitmap",     # 73
    "QCursor",     # 74
]

qdumpHelper_QVariants_F = [
    # Qt 5. In Qt 4 add one.
    "QKeySequence",# 75
    "QPen",        # 76
    "QTextLength", # 77
    "QTextFormat", # 78
    "X",
    "QTransform",  # 80
    "QMatrix4x4",  # 81
    "QVector2D",   # 82
    "QVector3D",   # 83
    "QVector4D",   # 84
    "QQuaternion", # 85
    "QPolygonF"    # 86
]

def qdump__QVariant(d, value):
    variantType = int(value["d"]["type"])
    #warn("VARIANT TYPE: %s : " % variantType)

    # Well-known simple type.
    if variantType <= 6:
        blob = d.toBlob(value)
        qdumpHelper_QVariants_A[variantType](d, blob)
        d.putNumChild(0)
        return None

    # Extended Core type (Qt 5)
    if variantType >= 31 and variantType <= 38 and d.qtVersion() >= 0x050000:
        blob = d.toBlob(value)
        qdumpHelper_QVariants_D[variantType - 31](d, blob)
        d.putNumChild(0)
        return None

    # Extended Core type (Qt 4)
    if variantType >= 128 and variantType <= 135 and d.qtVersion() < 0x050000:
        if variantType == 128:
            p = d.extractPointer(value)
            d.putBetterType("%sQVariant (void *)" % d.qtNamespace())
            d.putValue("0x%x" % p)
        else:
            if variantType == 135:
                blob = d.toBlob(value)
            else:
                p = d.extractPointer(value)
                p = d.extractPointer(p)
                blob = d.extractBlob(p, 8)
            qdumpHelper_QVariants_D[variantType - 128](d, blob)
        d.putNumChild(0)
        return None

    if variantType <= 86:
        # Known Core or Gui type.
        if variantType <= 28:
            innert = qdumpHelper_QVariants_B[variantType - 7]
        elif variantType <= 74:
            innert = qdumpHelper_QVariants_E[variantType - 64]
        elif d.qtVersion() < 0x050000:
            innert = qdumpHelper_QVariants_F[variantType - 76]
        else:
            innert = qdumpHelper_QVariants_F[variantType - 75]

        data = value["d"]["data"]
        ns = d.qtNamespace()
        inner = ns + innert
        if d.isLldb:
            # Looking up typedefs is problematic.
            if innert == "QVariantMap":
                inner = "%sQMap<%sQString, %sQVariant>" % (ns, ns, ns)
            elif innert == "QVariantHash":
                inner = "%sQHash<%sQString, %sQVariant>" % (ns, ns, ns)
            elif innert == "QVariantList":
                inner = "%sQList<%sQVariant>" % (ns, ns)
        innerType = d.lookupType(inner)

        if toInteger(value["d"]["is_shared"]):
            val = data["ptr"].cast(innerType.pointer().pointer()).dereference().dereference()
        else:
            val = data["ptr"].cast(innerType)

        d.putEmptyValue(-99)
        d.putItem(val)
        d.putBetterType("%sQVariant (%s)" % (d.qtNamespace(), innert))

        return innert


    # User types.
    d_ptr = value["d"]
    typeCode = int(d_ptr["type"])
    ns = d.qtNamespace()
    try:
        exp = "((const char *(*)(int))%sQMetaType::typeName)(%d)" % (ns, typeCode)
        type = str(d.parseAndEvaluate(exp))
    except:
        exp = "%sQMetaType::typeName(%d)" % (ns, typeCode)
        type = str(d.parseAndEvaluate(exp))
    type = type[type.find('"') + 1 : type.rfind('"')]
    type = type.replace("Q", ns + "Q") # HACK!
    type = type.replace("uint", "unsigned int") # HACK!
    type = type.replace("COMMA", ",") # HACK!
    type = type.replace(" ,", ",") # Lldb
    #warn("TYPE: %s" % type)
    data = d.call(value, "constData")
    #warn("DATA: %s" % data)
    d.putEmptyValue(-99)
    d.putType("%sQVariant (%s)" % (ns, type))
    d.putNumChild(1)
    tdata = data.cast(d.lookupType(type).pointer()).dereference()
    if d.isExpanded():
        with Children(d):
            with NoAddress(d):
                d.putSubItem("data", tdata)
    return tdata.type


def qedit__QVector(d, value, data):
    values = data.split(',')
    size = len(values)
    d.call(value, "resize", str(size))
    innerType = d.templateArgument(value.type, 0)
    try:
        # Qt 5. Will fail on Qt 4 due to the missing 'offset' member.
        offset = value["d"]["offset"]
        base = d.pointerValue(value["d"].cast(d.charPtrType()) + offset)
    except:
        # Qt 4.
        base = d.pointerValue(value["p"]["array"])
    d.setValues(base, innerType, values)


def qform__QVector():
    return arrayForms()


def qdump__QVector(d, value):
    data, size, alloc = d.vectorDataHelper(d.extractPointer(value))
    d.check(0 <= size and size <= alloc and alloc <= 1000 * 1000 * 1000)
    d.putItemCount(size)
    d.putNumChild(size)
    innerType = d.templateArgument(value.type, 0)
    d.putPlotData(innerType, data, size)


def qdump__QWeakPointer(d, value):
    d_ptr = value["d"]
    val = value["value"]
    if d.isNull(d_ptr) and d.isNull(val):
        d.putValue("(null)")
        d.putNumChild(0)
        return
    if d.isNull(d_ptr) or d.isNull(val):
        d.putValue("<invalid>")
        d.putNumChild(0)
        return
    weakref = int(d_ptr["weakref"]["_q_value"])
    strongref = int(d_ptr["strongref"]["_q_value"])
    d.check(strongref >= -1)
    d.check(strongref <= weakref)
    d.check(weakref <= 10*1000*1000)

    innerType = d.templateArgument(value.type, 0)
    if d.isSimpleType(innerType):
        d.putSimpleValue(val.dereference())
    else:
        d.putEmptyValue()

    d.putNumChild(3)
    if d.isExpanded():
        with Children(d):
            d.putSubItem("data", val.dereference().cast(innerType))
            d.putIntItem("weakref", weakref)
            d.putIntItem("strongref", strongref)


def qdump__QXmlAttributes(d, value):
    qdump__QList(d, value["attList"])


#######################################################################
#
# V4
#
#######################################################################

def qdump__QV4__String(d, value):
    d.putStringValue(value["identifier"]["string"])
    d.putNumChild(0)

def qdump__QV4__TypedValue(d, value):
    qdump__QV4__Value(d, d.directBaseObject(value))
    d.putBetterType(value.type)

def qdump__QV4__Value(d, value):
    try:
        if d.is64bit():
            vtable = value["m"]["internalClass"]["vtable"]
            if toInteger(vtable["isString"]):
                d.putBetterType(d.qtNamespace() + "QV4::Value (String)")
                d.putStringValue(value["s"]["identifier"]["string"])
                d.putNumChild(0)
                return
    except:
        pass

    # Fall back for cases that we do not handle specifically.
    d.putPlainChildren(value)


#######################################################################
#
# Webkit
#
#######################################################################


def jstagAsString(tag):
    # enum { Int32Tag =        0xffffffff };
    # enum { CellTag =         0xfffffffe };
    # enum { TrueTag =         0xfffffffd };
    # enum { FalseTag =        0xfffffffc };
    # enum { NullTag =         0xfffffffb };
    # enum { UndefinedTag =    0xfffffffa };
    # enum { EmptyValueTag =   0xfffffff9 };
    # enum { DeletedValueTag = 0xfffffff8 };
    if tag == -1:
        return "Int32"
    if tag == -2:
        return "Cell"
    if tag == -3:
        return "True"
    if tag == -4:
        return "Null"
    if tag == -5:
        return "Undefined"
    if tag == -6:
        return "Empty"
    if tag == -7:
        return "Deleted"
    return "Unknown"



def qdump__QTJSC__JSValue(d, value):
    d.putEmptyValue()
    d.putNumChild(1)
    if d.isExpanded():
        with Children(d):
            tag = value["u"]["asBits"]["tag"]
            payload = value["u"]["asBits"]["payload"]
            #d.putIntItem("tag", tag)
            with SubItem(d, "tag"):
                d.putValue(jstagAsString(int(tag)))
                d.putNoType()
                d.putNumChild(0)

            d.putIntItem("payload", int(payload))
            d.putFields(value["u"])

            if tag == -2:
                cellType = d.lookupType("QTJSC::JSCell").pointer()
                d.putSubItem("cell", payload.cast(cellType))

            try:
                # FIXME: This might not always be a variant.
                delegateType = d.lookupType(d.qtNamespace() + "QScript::QVariantDelegate").pointer()
                delegate = scriptObject["d"]["delegate"].cast(delegateType)
                #d.putSubItem("delegate", delegate)
                variant = delegate["m_value"]
                d.putSubItem("variant", variant)
            except:
                pass


def qdump__QScriptValue(d, value):
    # structure:
    #  engine        QScriptEnginePrivate
    #  jscValue      QTJSC::JSValue
    #  next          QScriptValuePrivate *
    #  numberValue   5.5987310416280426e-270 myns::qsreal
    #  prev          QScriptValuePrivate *
    #  ref           QBasicAtomicInt
    #  stringValue   QString
    #  type          QScriptValuePrivate::Type: { JavaScriptCore, Number, String }
    #d.putEmptyValue()
    dd = value["d_ptr"]["d"]
    ns = d.qtNamespace()
    if d.isNull(dd):
        d.putValue("(invalid)")
        d.putNumChild(0)
        return
    if int(dd["type"]) == 1: # Number
        d.putValue(dd["numberValue"])
        d.putType("%sQScriptValue (Number)" % ns)
        d.putNumChild(0)
        return
    if int(dd["type"]) == 2: # String
        d.putStringValue(dd["stringValue"])
        d.putType("%sQScriptValue (String)" % ns)
        return

    d.putType("%sQScriptValue (JSCoreValue)" % ns)
    x = dd["jscValue"]["u"]
    tag = x["asBits"]["tag"]
    payload = x["asBits"]["payload"]
    #isValid = int(x["asBits"]["tag"]) != -6   # Empty
    #isCell = int(x["asBits"]["tag"]) == -2
    #warn("IS CELL: %s " % isCell)
    #isObject = False
    #className = "UNKNOWN NAME"
    #if isCell:
    #    # isCell() && asCell()->isObject();
    #    # in cell: m_structure->typeInfo().type() == ObjectType;
    #    cellType = d.lookupType("QTJSC::JSCell").pointer()
    #    cell = payload.cast(cellType).dereference()
    #    dtype = "NO DYNAMIC TYPE"
    #    try:
    #        dtype = cell.dynamic_type
    #    except:
    #        pass
    #    warn("DYNAMIC TYPE: %s" % dtype)
    #    warn("STATUC  %s" % cell.type)
    #    type = cell["m_structure"]["m_typeInfo"]["m_type"]
    #    isObject = int(type) == 7 # ObjectType;
    #    className = "UNKNOWN NAME"
    #warn("IS OBJECT: %s " % isObject)

    #inline bool JSCell::inherits(const ClassInfo* info) const
    #for (const ClassInfo* ci = classInfo(); ci; ci = ci->parentClass) {
    #    if (ci == info)
    #        return true;
    #return false;

    try:
        # This might already fail for "native" payloads.
        scriptObjectType = d.lookupType(ns + "QScriptObject").pointer()
        scriptObject = payload.cast(scriptObjectType)

        # FIXME: This might not always be a variant.
        delegateType = d.lookupType(ns + "QScript::QVariantDelegate").pointer()
        delegate = scriptObject["d"]["delegate"].cast(delegateType)
        #d.putSubItem("delegate", delegate)

        variant = delegate["m_value"]
        #d.putSubItem("variant", variant)
        t = qdump__QVariant(d, variant)
        # Override the "QVariant (foo)" output
        d.putBetterType("%sQScriptValue (%s)" % (ns, t))
        if t != "JSCoreValue":
            return
    except:
        pass

    # This is a "native" JSCore type for e.g. QDateTime.
    d.putValue("<native>")
    d.putNumChild(1)
    if d.isExpanded():
        with Children(d):
           d.putSubItem("jscValue", dd["jscValue"])


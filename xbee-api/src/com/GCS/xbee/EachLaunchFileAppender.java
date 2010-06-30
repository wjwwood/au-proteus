package com.GCS.xbee;

import java.io.*;
import java.util.*;

import org.apache.log4j.*;
import org.apache.log4j.helpers.*;
import org.apache.log4j.spi.*;

/**
 * Provides a log file per execution.
 * The processing is done during initilaisation.
 * We check if a log file of the same name already exists, if so, we back the previous one as "logfilename.1".
 * We can keep track of several back ups (see the maxLogArchives property), by shiffting their number
 * (ie if "logfilename.1" exists, rename to "logfilename.2" and "logfilename" becomes "logfilename.1" and we log
 * to "logfilename" ).
 *
 * cf http://news.gmane.org/find-root.php?message_id=%3cc83e39890610250805l1ca82c4v3cbbd223b83f13d8%40mail.gmail.com%3e
 *
 * <pre>
 * log4j.rootLogger=DEBUG, exeroll
 * log4j.appender.exeroll=EachLaunchFileAppender
 * log4j.appender.exeroll.File=./log/client.log
 * # optional, default is 5
 * log4j.appender.exeroll.maxLogArchives=10
 * log4j.appender.exeroll.layout=org.apache.log4j.PatternLayout
 * log4j.appender.exeroll.layout.ConversionPattern=%d{ABSOLUTE} [%t] [%-5p] [%-25.48c{1}] - %m - [%l] %n
 * </pre>
 *
 * This is just a quick hack, it works for me, do whatever you want with it
 * Date: 25 oct. 2006
 *
 * @author Chris Dillon
 */
public class EachLaunchFileAppender extends FileAppender {

        private int maxLogArchives = 5;

        public EachLaunchFileAppender() {
        }

        public EachLaunchFileAppender(Layout pLayout, String filename) throws IOException {
                super(pLayout, filename);
        }

        public EachLaunchFileAppender(Layout pLayout, String filename, boolean append) throws IOException {
                super(pLayout, filename, append);
        }

        public EachLaunchFileAppender(Layout pLayout, String filename, boolean append, boolean pBufferedIO,
                                                                  int pBufferSize) throws IOException {
                super(pLayout, filename, append, pBufferedIO, pBufferSize);
        }

        public void activateOptions() {
                if (fileName != null) {
                        try {
                                handleBackup();
                                // Super processing
                                setFile(fileName, fileAppend, bufferedIO, bufferSize);
                        }
                        catch (IOException e) {
                                errorHandler.error("setFile(" + fileName + "," + fileAppend + ") call failed.",
                                                e, ErrorCode.FILE_OPEN_FAILURE);
                        }
                }
                else {
                        LogLog.warn("File option not set for appender [" + name + "].");
                        LogLog.warn("Are you using FileAppender instead of ConsoleAppender?");
                }
        }

        /**
         * If the file (fileName) to log to already exists, we try to back it up to fileName.1.
         * <br>
         * If previous backups already exist, we rename each of them to n+1 (ie: log.1 becomes log.2 etc...)
         * until we reache the value set by {@link #maxLogArchives} of log archives (we delete the oldest).
         */
        private void handleBackup() {
                final File newLogFile = new File(fileName);
                if (!newLogFile.exists()) {
                        return;
                }
                final File dir = newLogFile.getParentFile();
                final String logFileName = newLogFile.getName();
                String[] oldLogNames = dir.list(new FilenameFilter() {
                        String dotName = logFileName + ".";
                        public boolean accept(File pDir, String pName) {
                                return pName.startsWith(dotName);
                        }
                });
                // go through names from older to younger
                Arrays.sort(oldLogNames,new ArchiveComparator());
                for (int i = oldLogNames.length -1; i >= 0; i--) {
                        final String oldLogName = oldLogNames[i];
                        try {
                                int fileNo = getArchiveNumber(oldLogName);
                                if (fileNo >= maxLogArchives){
                                        LogLog.debug("deleting : "+oldLogName);
                                        File del = new File(dir,oldLogName);
                                        del.delete();
                                }
                                else{
                                        fileNo++;
                                        File toBackup = new File(dir, oldLogName);
                                        final long lastModif = toBackup.lastModified();
                                        final File backupTo = new File(fileName + "." + fileNo);
                                        LogLog.debug("Renamming " + toBackup + " to" + backupTo);
                                        boolean renOk = toBackup.renameTo(backupTo);
                                        backupTo.setLastModified(lastModif);
                                        LogLog.debug("Renamming " + toBackup + " to" + backupTo + " done :" + renOk);
                                }
                        }
                        catch (Throwable e) {
                                LogLog.warn("Error during back up of "+oldLogName,e);
                        }
                }
                final long lastModif = newLogFile.lastModified();
                final File backupTo = new File(fileName + ".1");
                LogLog.debug("Renamming " + newLogFile + " to" + backupTo);
                boolean renOk = newLogFile.renameTo(backupTo);
                LogLog.debug("Renamming " + newLogFile + " to" + backupTo + " done :" + renOk);
                backupTo.setLastModified(lastModif);
        }

        /**
         * returns the number of the archive.
         * log.2 return 2
         * @param pFileName
         * @return
         */
        private int getArchiveNumber(String pFileName){
                int lastDotIdx = pFileName.lastIndexOf('.');
                String oldLogNumber = pFileName.substring(lastDotIdx + 1);
                return Integer.parseInt(oldLogNumber);
        }

        private class ArchiveComparator implements Comparator<String> {
                public int compare(String p1, String p2) {
                        final int one = getArchiveNumber(p1);
                        final int two = getArchiveNumber(p2);
                        return one - two;
                }
        }



        public int getMaxLogArchives() {
                return maxLogArchives;
        }

        public void setMaxLogArchives(int pMaxLogArchives) {
                maxLogArchives = pMaxLogArchives;
        }

}

ADMSDIR = ../../..

XyceADMSFILES = $(ADMSDIR)/adms.implicit.xml \
	$(ADMSDIR)/xyceVersion_nosac.xml \
	$(ADMSDIR)/xyceBasicTemplates_nosac.xml \
	$(ADMSDIR)/xyceAnalogFunction_nosac.xml \
	$(ADMSDIR)/xyceHeaderFile_nosac.xml \
	$(ADMSDIR)/xyceImplementationFile_nosac.xml \
	$(ADMSDIR)/xyceOutVarsDoc.xml

BSIMCMGDEPFILES =bsimcmg_binning_parameters.include \
	bsimcmg_body.include \
	bsimcmg_cfringe.include \
	bsimcmg_compute_nqsmod3_segment.include \
	bsimcmg_load_nqsmod3_segments.include \
	bsimcmg_nqsmod3_iv_cv.include \
	bsimcmg_quasi_static_cv.include \
	bsimcmg_rdsmod.include \
	common_defs.include 

CLEANFILES = N_DEV_ADMSbsimcmg.h N_DEV_ADMSbsimcmg.C disciplines.vams constants.vams *.tex *.orig *.rej

all-source: N_DEV_ADMSbsimcmg.C

clean:
	rm -f $(CLEANFILES)

N_DEV_ADMSbsimcmg.h N_DEV_ADMSbsimcmg.C: bsimcmg.va $(XyceADMSFILES) $(BSIMCMGDEPFILES) insert_devconmap.patch
	admsXml -x -e $(ADMSDIR)/adms.implicit.xml -e $(ADMSDIR)/xyceVersion_nosac.xml -e $(ADMSDIR)/xyceBasicTemplates_nosac.xml -e $(ADMSDIR)/xyceAnalogFunction_nosac.xml -e $(ADMSDIR)/xyceImplementationFile_nosac.xml -e $(ADMSDIR)/xyceHeaderFile_nosac.xml -e $(ADMSDIR)/xyceOutVarsDoc.xml bsimcmg.va
	patch -l N_DEV_ADMSbsimcmg.C insert_devconmap.patch
#	emacs N_DEV_ADMSbsimcmg.C --batch --eval="(require 'cc-mode)" --eval="(c-set-offset 'substatement-open 0)" --eval="(c-set-offset 'arglist-intro 3)" --eval="(c-set-offset 'innamespace -2)" --eval="(setq-default indent-tabs-mode nil)" --eval='(indent-region (point-min) (point-max) nil)' -f save-buffer
#	emacs N_DEV_ADMSbsimcmg.h --batch --eval="(require 'cc-mode)" --eval="(c-set-offset 'substatement-open 0)" --eval="(c-set-offset 'arglist-intro 3)" --eval="(c-set-offset 'innamespace -2)" --eval="(setq-default indent-tabs-mode nil)" --eval='(indent-region (point-min) (point-max) nil)' -f save-buffer

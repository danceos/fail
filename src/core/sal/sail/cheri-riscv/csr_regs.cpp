
#define CSR_IMPL(CLS,REF) \
    IMPL_CAP_WRAP(CLS,REF) \
    SAIL_FN_REG(CLS)

CSR_IMPL(ddc,zDDC);
CSR_IMPL(utcc,zUTCC);
CSR_IMPL(utdc,zUTDC);
CSR_IMPL(uscratchc,zUScratchC);
CSR_IMPL(uepcc,zUEPCC);
CSR_IMPL(stcc,zSTCC);
CSR_IMPL(stdc,zSTDC);
CSR_IMPL(sscratchc,zSScratchC);
CSR_IMPL(sepcc,zSEPCC);
CSR_IMPL(mtcc,zMTCC);
CSR_IMPL(mtdc,zMTDC);
CSR_IMPL(mscratchc,zMScratchC);
CSR_IMPL(mepcc,zMEPCC);

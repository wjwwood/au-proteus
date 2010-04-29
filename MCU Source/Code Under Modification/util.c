short my_avs(unsigned short a, unsigned short b){
  
  short sa, sb;
  sa = (short) a;
  sb = (short) b;
  if( (sa-sb) < 0) return sb-sa;
  else return sa-sb;
}
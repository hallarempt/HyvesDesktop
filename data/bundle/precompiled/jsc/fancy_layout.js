var SMILEY_EXTRA_DEFAULT = 0;
var SMILEY_EXTRA_OPTIN = 1;
var SMILEY_EXTRA_PREMIUM = 2;

var smileyTrie = {'(':{'a':{')':{'\x00':['innocent','0',[7],'(a)']}},'h':{')':{'\x00':['bril','0',[9],'(h)']}},'k':{')':{'\x00':['love_kissing','0',[1],'(k)']}},'l':{')':{'\x00':['love_heart','0',[1],'(l)']}},'u':{')':{'\x00':['love_brokenheart','0',[1],'(u)']}},'6':{')':{'\x00':['duivels','0',[9],'(6)']}},'y':{')':{'\x00':['happy_thumbup','0',[2],'(y)']}},'^':{')':{'\x00':['happy_birthday','0',[5],'(^)']}}},'+':{'o':{'(':{'\x00':['sick','0',[5],'+o(']}}},'x':{'p':{'p':{'p':{'\x00':['tongue_fierce','0',[10],'xppp']}}},'-':{'p':{'p':{'p':{'\x00':['tongue_fierce','0',[10],'x-ppp']}}}}},'8':{'-':{'o':{'\x00':['duh','0',[6],'8-o']}},'o':{'\x00':['duh','0',[6],'8o']}},';':{')':{'\x00':['knipoog','0',[9],';)']},'-':{')':{'\x00':['knipoog','0',[9],';-)']}}},':':{'#':{'\x00':['geheim','0',[6],':#']},'$':{'\x00':['schamen','0',[9],':$']},"'":{'(':{'\x00':['cry','0',[9,6],":'("]},'-':{'(':{'\x00':['cry','0',[9,6],":'-("]}}},')':{'\x00':['lach','0',[9],':)']},'(':{'\x00':['verdrietig','0',[9],':('],'(':{')':{'\x00':['depressive','0',[6],':(()']}}},'-':{'@':{'\x00':['boos','0',[9],':-@']},'#':{'\x00':['geheim','0',[6],':-#']},'d':{'\x00':['groot','0',[9],':-d'],'p':{'\x00':['tongue_updown','0',[10],':-dp']}},')':{'\x00':['lach','0',[9],':-)']},'(':{'\x00':['verdrietig','0',[9],':-(']},'o':{'\x00':['surprised','0',[9],':-o']},'p':{'\x00':['tong2','0',[9],':-p'],'&':{'g':{'t':{';':{'\x00':['tongue_sleepout','0',[10],':-p&gt;']}}}},'p':{'p':{'\x00':['tongue_fingersear','0',[10],':-ppp']}}},'s':{'\x00':['indewar','0',[9],':-s']},'|':{'\x00':['geschokt','0',[9],':-|']},'?':{'\x00':['confused_vraagteken','0',[6],':-?']},'$':{'\x00':['schamen','0',[9],':-$']}},'?':{'\x00':['confused_vraagteken','0',[6],':?']},'@':{'\x00':['boos','0',[9],':@']},'a':{'c':{'o':{'l':{'d':{':':{'\x00':['acold','0',[8]]}}}}},'d':{'f':{'u':{'n':{'d':{'u':{'m':{':':{'\x00':['adfundum','2',[5]]}}}}}}}},'h':{'h':{':':{'\x00':['ahh','2',[1]]}}},'l':{'l':{'i':{'n':{':':{'\x00':['allin','0',[12]]}}}}},'n':{'g':{'r':{'y':{'g':{'i':{'r':{'l':{':':{'\x00':['angrygirl','2',[3]]}}}}}}},'e':{'l':{':':{'\x00':['angel','0',[7]]},'m':{'u':{'s':{'i':{'c':{':':{'\x00':['angelmusic','1',[5]]}}}}}},'l':{'o':{'v':{'e':{':':{'\x00':['angellove','0',[1]]}}}}}}}}},'r':{'r':{'o':{'g':{'a':{'n':{'t':{':':{'\x00':['arrogant','2',[10]]}}}}}}},'m':{'s':{'c':{'r':{'o':{'s':{'s':{'e':{'d':{':':{'\x00':['armscrossed','2',[3]]}}}}}}}}}}},'t':{'t':{'e':{'n':{'t':{'i':{'o':{'n':{':':{'\x00':['attention','0',[7]]}}}}}}}}}},'c':{'a':{'t':{':':{'\x00':['cat','0',[11]]}},'r':{'a':{'v':{'a':{'n':{':':{'\x00':['caravan','2',[8]]}}}}},'n':{'a':{'v':{'a':{'l':{':':{'\x00':['carnaval','1',[5]]}}}}}}},'l':{'l':{':':{'\x00':['call','2',[8]]}}},'n':{'d':{'l':{'e':{':':{'\x00':['candle','2',[5]]}}}}}},'i':{'g':{'a':{'r':{':':{'\x00':['cigar','2',[4]]}}}}},'h':{'i':{'c':{'k':{':':{'\x00':['chick','2',[11]]},'e':{'n':{':':{'\x00':['chicken','2',[11]]}}}}},'t':{'c':{'h':{'a':{'t':{':':{'\x00':['chitchat','1',[5]]}}}}}},'n':{'r':{'u':{'b':{':':{'\x00':['chinrub','0',[6]]}}}}}},'a':{'m':{'p':{':':{'\x00':['champ','2',[5]]}}}},'e':{'e':{'s':{'e':{':':{'\x00':['cheese','2',[2]]}}},'r':{'i':{'n':{'g':{':':{'\x00':['cheering','0',[5]]}}}},'s':{':':{'\x00':['cheers','0',[5]]}},':':{'\x00':['cheer','0',[2]]},'l':{'e':{'a':{'d':{'e':{'r':{'s':{':':{'\x00':['cheerleaders','2',[5]]}},':':{'\x00':['cheerleader','0',[5]]}}}}}}}}}}},'l':{'a':{'p':{':':{'\x00':['clap','0',[2]]}}},'e':{'a':{'n':{'i':{'n':{'g':{':':{'\x00':['cleaning','2',[8]]}}}}}}},'o':{'u':{'d':{'n':{'i':{'n':{'e':{':':{'\x00':['cloudnine','2',[1]]}}}}}}},'w':{'n':{':':{'\x00':['clown','0',[7]]}}}}},'o':{'c':{'k':{'t':{'a':{'i':{'l':{':':{'\x00':['cocktail','1',[5]]}}}}}}},'f':{'f':{'e':{'e':{':':{'\x00':['coffee','0',[8]]}}}}},'m':{'p':{'u':{'t':{'e':{'r':{'f':{'i':{'g':{'h':{'t':{':':{'\x00':['computerfight','0',[12]]}}}}}}}}}}}},'o':{'k':{':':{'\x00':['cook','0',[7]]}},'l':{'p':{'e':{'a':{'c':{'e':{':':{'\x00':['coolpeace','0',[4]]}}}}}}}},'u':{'c':{'h':{':':{'\x00':['couch','2',[8]]}}}},'w':{':':{'\x00':['cow','1',[11]]},'b':{'o':{'y':{':':{'\x00':['cowboy','0',[3]]}}}}}},'r':{'a':{'z':{'y':{':':{'\x00':['crazy','0',[2]]}}}},'u':{'s':{'h':{':':{'\x00':['crush','0',[1]]}}}},'o':{'c':{'o':{'d':{'i':{'l':{'e':{':':{'\x00':['crocodile','0',[12]]}}}}}}}}},'u':{'p':{'i':{'d':{':':{'\x00':['cupid','1',[1]]}}}}}},'b':{'a':{'b':{'y':{':':{'\x00':['baby','0',[7]]}},'e':{':':{'\x00':['babe','0',[7]]}}},'d':{'m':{'o':{'o':{'d':{':':{'\x00':['badmood','2',[3]]}}}}}},'g':{'u':{'e':{'t':{'t':{'e':{':':{'\x00':['baguette','1',[7]]}}}}}},'o':{'n':{'h':{'e':{'a':{'d':{':':{'\x00':['bagonhead','0',[7]]}}}}}}}},'n':{'a':{'n':{'a':{':':{'\x00':['banana','0',[5]]}}}}},'s':{'k':{'e':{'t':{'b':{'a':{'l':{'l':{':':{'\x00':['basketball','2',[8]]}}}}}}}},'e':{'b':{'a':{'l':{'l':{':':{'\x00':['baseball','0',[7]]}}}}}}},'t':{'h':{'i':{'n':{'g':{':':{'\x00':['bathing','0',[8]]}}}}}}},'e':{'a':{'r':{'d':{':':{'\x00':['beard','0',[4]]}}}},'s':{'t':{'f':{'r':{'i':{'e':{'n':{'d':{'s':{':':{'\x00':['bestfriends','2',[1]]}}}}}}}}}},'e':{':':{'\x00':['bee','0',[11]]}},'l':{'l':{'y':{'d':{'a':{'n':{'c':{'e':{'r':{':':{'\x00':['bellydancer','0',[7]]}}}}}}}}}}},')':{'\x00':['bril','0',[9],':b)']},'-':{')':{'\x00':['bril','0',[9],':b-)']}},'l':{'a':{'c':{'k':{'e':{'y':{'e':{':':{'\x00':['blackeye','0',[6]]}}}}}},'b':{'l':{'a':{'b':{'l':{'a':{':':{'\x00':['blablabla','0',[3]]}}}}}}}},'i':{'n':{'k':{':':{'\x00':['blink','0',[4]]}}}}},'o':{'x':{'i':{'n':{'g':{':':{'\x00':['boxing','1',[8]]}}}}},'r':{'i':{'s':{':':{'\x00':['boris','0',[7]]}}},'e':{'d':{':':{'\x00':['bored','0',[8]]}}}},'d':{'y':{'g':{'u':{'a':{'r':{'d':{':':{'\x00':['bodyguard','2',[7]]}}}}}}}},'o':{'g':{'i':{'e':{':':{'\x00':['boogie','0',[4]]}}}}},'n':{'d':{':':{'\x00':['bond','2',[4]]}}}},'i':{'k':{'e':{':':{'\x00':['bike','0',[8]]}}},'r':{'d':{':':{'\x00':['bird','2',[11]]}}},'g':{'n':{'e':{'w':{'s':{':':{'\x00':['bignews','0',[8]]}}}}}}},'r':{'a':{'n':{'c':{'a':{'r':{'d':{':':{'\x00':['brancard','0',[8]]}}}}}}},'u':{'s':{'h':{'i':{'n':{'g':{':':{'\x00':['brushing','0',[8]]}}}}}}},'e':{'a':{'k':{'d':{'a':{'n':{'c':{'e':{':':{'\x00':['breakdance','0',[5]]}}}}}}}},'n':{'d':{'a':{'h':{'e':{'n':{'d':{'r':{'i':{'k':{'s':{':':{'\x00':['giggle','2',[1],':brendahendriks:']}}}}}}}}}}}}},'o':{':':{'\x00':['bro','0',[4]]}}},'u':{'t':{'t':{'e':{'r':{'f':{'l':{'y':{':':{'\x00':['butterfly','0',[11]]}}}}}}}}},'y':{'e':{'b':{'y':{'e':{':':{'\x00':['byebye','0',[1]]}}}}}}},'e':{'a':{'t':{'p':{'a':{'p':{'e':{'r':{':':{'\x00':['eatpaper','0',[8]]}}}}}},'c':{'r':{'o':{'c':{'o':{'d':{'i':{'l':{'e':{':':{'\x00':['eatcrocodile','1',[3]]}}}}}}}}}}}},'g':{'y':{'p':{'t':{'i':{'a':{'n':{':':{'\x00':['egyptian','2',[5]]}}}}}}},'o':{'b':{'o':{'o':{'s':{'t':{':':{'\x00':['egoboost','0',[12]]}}}}}}}},'l':{'e':{'p':{'h':{'a':{'n':{'t':{':':{'\x00':['elephant','0',[11]]},'n':{'o':{'s':{'e':{':':{'\x00':['elephantnose','2',[11]]}}}}}}}}}}}},'u':{'f':{':':{'\x00':['hyvesangel','2',[12],':euf:']}}},'t':{':':{'\x00':['et','2',[7]]}},'v':{'i':{'l':{'l':{'o':{'o':{'k':{':':{'\x00':['evillook','2',[10]]}}}}}}}},'y':{'e':{'s':{':':{'\x00':['eyes','0',[6]]}}}}},'d':{'a':{'n':{'c':{'i':{'n':{'g':{':':{'\x00':['dancing','0',[5]]}}}}}}},'\x00':['groot','0',[9],':d'],'e':{'a':{'d':{':':{'\x00':['dead','0',[3]]}}},'e':{'r':{':':{'\x00':['deer','1',[11]]}}},'v':{'i':{'l':{':':{'\x00':['devil','0',[3]]},'b':{'a':{'b':{'e':{':':{'\x00':['devilbabe','0',[7]]}}}}}}}}},'i':{'s':{'a':{'p':{'p':{'e':{'a':{'r':{':':{'\x00':['disappear','1',[3]]}}}}}}},'c':{'o':{':':{'\x00':['disco','0',[5]]}}}},'l':{'e':{'m':{'m':{'a':{':':{'\x00':['dilemma','2',[6]]}}}}}},'n':{'n':{'e':{'r':{':':{'\x00':['dinner','0',[1]]}}}}}},'j':{':':{'\x00':['dj','0',[5]]}},'o':{'i':{'k':{'n':{'o':{'w':{'y':{'o':{'u':{':':{'\x00':['doiknowyou','0',[4]]}}}}}}}}},'c':{'t':{'o':{'r':{':':{'\x00':['doctor','1',[7]]}}}}},'g':{':':{'\x00':['dog','0',[11]]}}},'p':{'\x00':['tongue_updown','0',[10],':dp']},'r':{'i':{'v':{'i':{'n':{'g':{':':{'\x00':['driving','0',[8]]}}}}},'n':{'k':{'i':{'n':{'g':{':':{'\x00':['drinking','0',[5]]}}}}}}},'a':{'w':{'i':{'n':{'g':{':':{'\x00':['drawing','0',[8]]}}}}}},'u':{'n':{'k':{'o':{'n':{'c':{'h':{'a':{'i':{'r':{':':{'\x00':['drunkonchair','2',[8]]}}}}}}}}}}}},'u':{'c':{'k':{':':{'\x00':['duck','0',[11]]}}}}},'g':{'e':{'t':{'a':{'l':{'i':{'f':{'e':{':':{'\x00':['getalife','0',[12]]}}}}}}},'n':{'t':{'l':{'e':{'m':{'a':{'n':{':':{'\x00':['gentleman','2',[7]]}}}}}}}}},'i':{'r':{'l':{'g':{'i':{'g':{'g':{'l':{'e':{':':{'\x00':['girlgiggle','2',[7]]}}}}}}}}},'g':{'g':{'l':{'e':{':':{'\x00':['giggle','2',[1]]}}}}}},'h':{'o':{'s':{'t':{':':{'\x00':['ghost','0',[7]]}}}}},'o':{'a':{'w':{'a':{'y':{':':{'\x00':['goaway','0',[3]]}}}}},'o':{'d':{'b':{'y':{'e':{':':{'\x00':['goodbye','1',[6]]}}}},'n':{'i':{'g':{'h':{'t':{':':{'\x00':['goodnight','2',[8]]}}}}}}}}},'s':{'m':{':':{'\x00':['gsm','2',[8]]}}},'r':{'e':{'a':{'s':{'e':{':':{'\x00':['grease','2',[4]]}}}}},'o':{'u':{'p':{'h':{'u':{'g':{':':{'\x00':['grouphug','0',[1]]}}}},'d':{'a':{'n':{'c':{'e':{':':{'\x00':['groupdance','2',[5]]}}}}}}}}}},'u':{'i':{'l':{'l':{'o':{'t':{'i':{'n':{'e':{':':{'\x00':['guillotine','1',[3]]}}}}}}}}}}},'f':{'a':{'r':{'m':{'e':{'r':{':':{'\x00':['farmer','2',[7]]}}}},'e':{'w':{'e':{'l':{'l':{':':{'\x00':['farewell','2',[1]]}}}}}},'t':{':':{'\x00':['fart','2',[8]]}}}},'e':{'v':{'e':{'r':{':':{'\x00':['fever','0',[6]]}}}}},'i':{'s':{'h':{'i':{'n':{'g':{':':{'\x00':['fishing','0',[8]]}}}}},'t':{'s':{':':{'\x00':['fists','2',[3]]}}}},'r':{'s':{'t':{'d':{'a':{'t':{'e':{':':{'\x00':['firstdate','1',[1]]}}}}}}},'e':{':':{'\x00':['fire','2',[8]]},'f':{'o':{'x':{':':{'\x00':['firefox','0',[12]]}}}}}},'t':{'n':{'e':{'s':{'s':{':':{'\x00':['fitness','0',[8]]}}}}}}},'l':{'a':{'b':{'b':{'e':{'r':{'g':{'a':{'s':{'t':{'e':{'d':{':':{'\x00':['flabbergasted','1',[10]]}}}}}}}}}}},'m':{'e':{'s':{':':{'\x00':['flames','2',[3]]}}}}},'y':{'i':{'n':{'g':{':':{'\x00':['flying','0',[8]]}}}}},'o':{'w':{'e':{'r':{'s':{':':{'\x00':['flowers','0',[1]]}},':':{'\x00':['flower','2',[2]]}}}}}},'o':{'x':{':':{'\x00':['fox','2',[11]]}},'r':{'m':{'u':{'l':{'a':{'1':{':':{'\x00':['formula1','2',[8]]}}}}}}},'o':{'l':{':':{'\x00':['fool','0',[7]]}}}},'r':{'e':{'e':{'z':{'i':{'n':{'g':{':':{'\x00':['freezing','0',[8]]}}}}}}},'u':{'i':{'t':{'c':{'a':{'k':{'e':{':':{'\x00':['fruitcake','0',[4]]}}}}}}}},'o':{'g':{':':{'\x00':['frog','0',[11]]}}}},'u':{'n':{'n':{'y':{'f':{'a':{'c':{'e':{':':{'\x00':['funnyface','1',[10]]}}}}}}}}}},'i':{'p':{'h':{'o':{'n':{'e':{'d':{'j':{':':{'\x00':['iphonedj','0',[12]]}}},'l':{'o':{'v':{'e':{':':{'\x00':['iphonelove','0',[12]]}}}}}}}}}},'r':{'e':{'n':{'e':{':':{'\x00':['hyvesangel','2',[12],':irene:']}}}},'o':{'n':{'i':{'n':{'g':{':':{'\x00':['ironing','0',[8]]}}}}}}},'n':{'d':{'i':{'a':{'n':{':':{'\x00':['indian','0',[7]]}}}}},'l':{'o':{'v':{'e':{':':{'\x00':['inlove','2',[1]]}}}}},'t':{'h':{'e':{'b':{'a':{'g':{':':{'\x00':['inthebag','2',[2]]}}}}}}}}},'h':{'a':{'p':{'p':{'y':{'g':{'r':{'o':{'u':{'p':{':':{'\x00':['happygroup','1',[5]]}}}}}}}}},'m':{'m':{'e':{'r':{':':{'\x00':['hammer','0',[3]]}}}}},'t':{'e':{'m':{'e':{'n':{':':{'\x00':['hatemen','2',[1]]}}}}}},'n':{'d':{'k':{'i':{'s':{'s':{':':{'\x00':['handkiss','0',[1]]}}}}}},'g':{'l':{'o':{'o':{'s':{'e':{':':{'\x00':['hangloose','0',[5]]}}}}}}}}},'e':{'a':{'r':{'t':{'s':{':':{'\x00':['hearts','0',[1]]}},'b':{'o':{'u':{'n':{'c':{'e':{':':{'\x00':['heartbounce','0',[1]]}}}}}}},'e':{'y':{'e':{'s':{':':{'\x00':['hearteyes','0',[1]]}}}}},'k':{'i':{'s':{'s':{':':{'\x00':['heartkiss','0',[12]]}}}}}}},'d':{'p':{'h':{'o':{'n':{'e':{'s':{':':{'\x00':['headphones','0',[7]]}}}}}}},'b':{'a':{'n':{'g':{':':{'\x00':['headbang','0',[5]]}}}}}}},'l':{'e':{'e':{'n':{':':{'\x00':['hyvesangel','2',[12],':heleen:']}}}}}},'i':{'p':{'p':{'i':{'e':{':':{'\x00':['hippie','1',[4]]}}}}},'t':{'c':{'h':{'h':{'i':{'k':{'e':{':':{'\x00':['hitchhike','0',[8]]}}}}}}}}},'o':{'c':{'k':{'e':{'y':{':':{'\x00':['hockey','0',[8]]}}}}},'s':{'a':{'n':{'n':{'a':{':':{'\x00':['hosanna','0',[2]]}}}}}},'r':{'s':{'e':{'r':{'i':{'d':{'e':{':':{'\x00':['horseride','2',[8]]}}}}}}}},'l':{'d':{'h':{'a':{'n':{'d':{'s':{':':{'\x00':['holdhands','0',[1]]}}}}}}},'l':{'a':{'n':{'d':{'y':{'e':{'l':{'l':{':':{'\x00':['hollandyell','0',[5]]}}}}},'s':{'c':{'h':{'m':{'i':{'n':{'k':{':':{'\x00':['hollandschmink','0',[5]]}}}}}}}},':':{'\x00':['holland','0',[5]]},'f':{'l':{'a':{'g':{':':{'\x00':['hollandflag','0',[5]]}}}}}}}}}}},'u':{'h':{':':{'\x00':['huh','1',[6]]}},'m':{'p':{'i':{'n':{'g':{':':{'\x00':['humping','0',[12]]}}}}}},'g':{'g':{'i':{'n':{'g':{':':{'\x00':['hugging','0',[1]]}}}}}},'n':{'g':{'r':{'y':{':':{'\x00':['hungry','0',[5]]}}}}}},'y':{'v':{'e':{'s':{'a':{'n':{'g':{'e':{'l':{':':{'\x00':['hyvesangel','2',[12]]}}}}}}}}}}},'k':{'i':{'s':{'s':{'i':{'n':{'g':{':':{'\x00':['kissing','2',[1]]}}}},'h':{'a':{'n':{'d':{':':{'\x00':['kisshand','0',[1]]}}}}},'b':{'l':{'u':{'s':{'h':{':':{'\x00':['kissblush','0',[1]]}}}}}}}},'e':{'k':{'e':{'b':{'o':{'e':{':':{'\x00':['kiekeboe','0',[8]]}}}}}}},'l':{'i':{'a':{'n':{':':{'\x00':['kilian','0',[5]]}}}}},'n':{'g':{':':{'\x00':['king','2',[7]]}}}},'n':{'i':{'g':{'h':{'t':{':':{'\x00':['knight','1',[7]]}}}}},'o':{'c':{'k':{'o':{'u':{'t':{':':{'\x00':['knockout','2',[6]]}}}}}}}}},'j':{'u':{'m':{'p':{'i':{'n':{'g':{':':{'\x00':['jumping','0',[2]]}}}}}},'g':{'g':{'l':{'i':{'n':{'g':{':':{'\x00':['juggling','0',[8]]}}}}}}}},'o':{'h':{'n':{'n':{'y':{':':{'\x00':['johnny','0',[7]]}}}}}}},'m':{'a':{'c':{'h':{'i':{'n':{'e':{'g':{'u':{'n':{':':{'\x00':['machinegun','0',[3]]}}}}}}}}},'r':{'l':{'i':{'n':{'d':{'e':{':':{'\x00':['XD','0',[2],':marlinde:']}}}}}},'t':{'i':{'n':{'i':{'b':{'a':{'b':{'y':{':':{'\x00':['martinibaby','2',[7]]}}}}}}}},'y':{':':{'\x00':['hyvesangel','2',[12],':marty:']}}}},'d':{':':{'\x00':['mad','0',[6]]}},'t':{'r':{'i':{'x':{':':{'\x00':['matrix','2',[7]]}}}}}},'i':{'c':{'k':{'e':{'y':{':':{'\x00':['mickey','0',[7]]}}}},'r':{'o':{'w':{'a':{'v':{'e':{':':{'\x00':['microwave','2',[8]]}}}}}}}},'r':{'r':{'o':{'r':{':':{'\x00':['lookinmirror','2',[7],':mirror:']}}}}},'d':{'d':{'l':{'e':{'f':{'i':{'n':{'g':{'e':{'r':{':':{'\x00':['middlefinger','0',[10]]}}}}}}}}}}}},'e':{'r':{'a':{'l':{':':{'\x00':['bellydancer','0',[7],':meral:']}}},'r':{'y':{'c':{'h':{'r':{'i':{'s':{'t':{'m':{'a':{'s':{':':{'\x00':['merrychristmas','0',[7]]}}}}}}}}}}}}}},'u':{'s':{'i':{'c':{':':{'\x00':['music','0',[8]]}}}}},'o':{'u':{'s':{'e':{':':{'\x00':['mouse','0',[11]]}}},'t':{'h':{':':{'\x00':['mouth','0',[6]]}}}},'o':{'n':{':':{'\x00':['moon','0',[12]]},'w':{'a':{'l':{'k':{':':{'\x00':['moonwalk','2',[5]]}}}}}}},'n':{'k':{'e':{'y':{'t':{'a':{'i':{'l':{':':{'\x00':['monkeytail','2',[11]]}}}}}}}}}}},'l':{'i':{'p':{'s':{':':{'\x00':['lips','0',[1]]}}},'k':{'e':{'u':{'a':{'l':{'o':{'t':{':':{'\x00':['likeualot','2',[1]]}}}}}}}},'n':{'s':{'e':{'y':{':':{'\x00':['hyvesangel','2',[12],':linsey:']}}}}},'c':{'k':{'i':{'n':{'g':{':':{'\x00':['licking','0',[10]]}}}}}},'f':{'t':{'i':{'n':{'g':{':':{'\x00':['lifting','0',[8]]}}}}}}},'e':{'k':{'k':{'e':{'r':{'d':{'i':{'n':{'g':{':':{'\x00':['lekkerding','0',[1]]}}}}}}}}}},'o':{'s':{'e':{'r':{':':{'\x00':['loser','2',[10]]}}}},'l':{':':{'\x00':['lol','0',[2]]},'l':{'y':{':':{'\x00':['lolly','2',[8]]}},'e':{'g':{'s':{':':{'\x00':['lollegs','2',[2]]}}}},'o':{'l':{':':{'\x00':['lollol','0',[2]]}}}},'o':{'n':{'g':{'r':{'o':{'u':{'n':{'d':{':':{'\x00':['lolonground','0',[2]]}}}}}}}}}},'v':{'e':{'y':{'o':{'u':{':':{'\x00':['loveyou','0',[1]]}}}},'s':{'h':{'y':{':':{'\x00':['loveshy','2',[1]]}}}},'m':{'a':{'i':{'l':{':':{'\x00':['lovemail','0',[12]]}}}}},'f':{'r':{'o':{'g':{':':{'\x00':['lovefrog','2',[1]]}}}}}}}}},'o':{'\x00':['surprised','0',[9],':o'],'n':{'e':{'g':{'l':{'a':{'s':{'s':{':':{'\x00':['oneglass','0',[7]]}}}}}}}}},'n':{'a':{'i':{'l':{'s':{':':{'\x00':['nails','2',[8]]}}}},'h':{':':{'\x00':['nah','0',[6]]}}},'i':{'c':{'o':{'l':{'a':{':':{'\x00':['bunny2','0',[12],':nicola:']}}}}},'n':{'j':{'a':{':':{'\x00':['ninja','0',[7]]}}},'e':{':':{'\x00':['hyvesangel','2',[12],':nine:']}}}},'e':{'r':{'d':{':':{'\x00':['nerd','0',[7]]}}},'d':{'e':{'r':{'w':{'i':{'e':{'t':{':':{'\x00':['weed','0',[12],':nederwiet:']}}}}}}}},'w':{'c':{'o':{'f':{'f':{'e':{'e':{':':{'\x00':['newcoffee','0',[2]]}}}}}}},'s':{'p':{'a':{'p':{'e':{'r':{':':{'\x00':['newspaper','1',[8]]}}}}}}}}},'u':{'r':{'s':{'e':{':':{'\x00':['nurse','2',[7]]}}}},'t':{'s':{':':{'\x00':['nuts','2',[7]]}}}},'o':{'n':{'o':{':':{'\x00':['nono','2',[6]]}}}}},'p':{'a':{'c':{'m':{'a':{'n':{':':{'\x00':['pacman','0',[7]]}}}}},'r':{'i':{'s':{'a':{':':{'\x00':['hyvesangel','2',[12],':parisa:']}}}},'t':{'y':{'a':{'n':{'i':{'m':{'a':{'l':{':':{'\x00':['partyanimal','0',[12]]}}}}}}},':':{'\x00':['party','0',[5]]}}}},'n':{'d':{'a':{':':{'\x00':['panda','2',[11]]}}}}},'\x00':['tong2','0',[9],':p'],'e':{'a':{'c':{'e':{':':{'\x00':['peace','0',[2]]}}}},'r':{'f':{'e':{'c':{'t':{':':{'\x00':['perfect','2',[2]]}}}}}}},'&':{'g':{'t':{';':{'\x00':['tongue_sleepout','0',[10],':p&gt;']}}}},'i':{'a':{'n':{'o':{':':{'\x00':['piano','0',[8]]}}}},'g':{':':{'\x00':['pig','0',[11]]}},'m':{'p':{':':{'\x00':['pimp','2',[7]]}}},'l':{'o':{'t':{':':{'\x00':['pilot','2',[7]]}}}},'p':{'e':{':':{'\x00':['pipe','0',[7]]}}},'r':{'a':{'t':{'e':{':':{'\x00':['pirate','0',[7]]}}}}},'z':{'z':{'a':{':':{'\x00':['pizza','1',[8]]}}}}},'h':{'o':{'n':{'e':{'f':{'u':{'n':{':':{'\x00':['phonefun','2',[2]]}}}}}}}},'l':{'a':{'y':{'i':{'n':{'g':{':':{'\x00':['playing','0',[8]]}}}},'c':{'a':{'r':{':':{'\x00':['playcar','0',[8]]}}}}}},'e':{'a':{'s':{'e':{':':{'\x00':['please','0',[6]]}}}}}},'o':{'p':{'c':{'o':{'r':{'n':{':':{'\x00':['popcorn','0',[5]]}}}}}},'k':{'e':{'r':{':':{'\x00':['poker','2',[8]]}}}},'e':{'t':{':':{'\x00':['poet','2',[8]]}}},'l':{'i':{'c':{'e':{':':{'\x00':['police','0',[7]]}}}}}},'p':{'p':{'\x00':['tongue_fingersear','0',[10],':ppp']}},'r':{'o':{'t':{'e':{'s':{'t':{':':{'\x00':['protest','0',[3]]}}}}}}}},'s':{'a':{'i':{'l':{'i':{'n':{'g':{':':{'\x00':['sailing','0',[8]]}}}}},'n':{'t':{':':{'\x00':['saint','2',[7]]}}}},'x':{':':{'\x00':['sax','0',[4]]}},'m':{'u':{'r':{'a':{'i':{':':{'\x00':['samurai','0',[7]]}}}}}},'l':{'e':{'s':{'m':{'a':{'n':{':':{'\x00':['salesman','1',[7]]}}}}}}},'n':{'t':{'a':{':':{'\x00':['santa','0',[7]]}}}}},'\x00':['indewar','0',[9],':s'],'c':{'h':{'a':{'d':{'e':{':':{'\x00':['schade','0',[5]]}}}}},'o':{'o':{'t':{'e':{'r':{':':{'\x00':['scooter','1',[8]]}}}}}}},'e':{'a':{'l':{'e':{'d':{':':{'\x00':['sealed','0',[6,7]]}}}}}},'i':{'n':{'t':{':':{'\x00':['sint','0',[7]]}},'g':{'i':{'n':{'g':{':':{'\x00':['singing','2',[8]]}}}}}}},'h':{'i':{'p':{'w':{'r':{'e':{'c':{'k':{'e':{'d':{':':{'\x00':['shipwrecked','2',[7]]}}}}}}}}},'t':{'h':{'a':{'p':{'p':{'e':{'n':{'s':{':':{'\x00':['shithappens','0',[7]]}}}}}}}}},'n':{'i':{'n':{'g':{':':{'\x00':['shining','1',[2]]}}}}}},'a':{'k':{'e':{'i':{'t':{':':{'\x00':['shakeit','0',[5]]}}}}},'d':{'e':{'s':{':':{'\x00':['shades','0',[4]]}}}}},'r':{'u':{'g':{':':{'\x00':['shrug','0',[6]]}}}},'e':{'r':{'l':{'o':{'c':{'k':{':':{'\x00':['sherlock','2',[7]]}}}}}}},'o':{'c':{'k':{':':{'\x00':['shock','0',[6]]}}},'u':{'t':{'i':{'n':{'g':{':':{'\x00':['shouting','0',[3]]}}}}}},'o':{'t':{':':{'\x00':['shoot','0',[3]]}}},'w':{'h':{'e':{'a':{'r':{'t':{':':{'\x00':['showheart','2',[1]]}}}}}}}}},'k':{'a':{'t':{'e':{'r':{':':{'\x00':['skater','1',[4]]}}}}},'y':{'w':{'a':{'l':{'k':{'e':{'r':{':':{'\x00':['skywalker','2',[7]]}}}}}}}},'i':{'i':{'n':{'g':{':':{'\x00':['skiing','2',[8]]}}}}}},'m':{'o':{'k':{'i':{'n':{'g':{':':{'\x00':['smoking','0',[4]]}}}}}}},'l':{'a':{'p':{'h':{'e':{'a':{'d':{':':{'\x00':['slaphead','2',[6]]}}}}},':':{'\x00':['slap','2',[3]]},'b':{'a':{'c':{'k':{'h':{'e':{'a':{'d':{':':{'\x00':['slapbackhead','2',[3]]}}}}}}}}},'f':{'a':{'c':{'e':{':':{'\x00':['slapface','2',[3]]}}}}}}},'e':{'e':{'p':{'i':{'n':{'g':{':':{'\x00':['sleeping','0',[8]]},'b':{'a':{'c':{'k':{':':{'\x00':['sleepingback','2',[8]]}}}}}}}}}}},'o':{'b':{'b':{'e':{'r':{':':{'\x00':['slobber','1',[2]]}}}}}}},'o':{'s':{':':{'\x00':['sos','0',[7]]}},'r':{'r':{'y':{':':{'\x00':['sorry','2',[6]]}}},'t':{'o':{'f':{':':{'\x00':['sortof','0',[2]]}}}}},'c':{'c':{'e':{'r':{':':{'\x00':['soccer','0',[8]]}}}}}},'n':{'o':{'r':{'k':{'l':{'i':{'n':{'g':{':':{'\x00':['snorkling','0',[7]]}}}}}}},'o':{'p':{'y':{':':{'\x00':['snoopy','1',[11]]}}}}}},'p':{'a':{'m':{':':{'\x00':['spam','0',[3]]}}},'i':{'d':{'e':{'r':{':':{'\x00':['spider','0',[11]]},'m':{'a':{'n':{':':{'\x00':['spiderman','2',[7]]}}}}}}}}},'u':{'p':{'p':{'o':{'r':{'t':{':':{'\x00':['support','2',[8]]}}}}},'e':{'r':{'m':{'a':{'n':{'2':{':':{'\x00':['superman2','0',[7]]}},':':{'\x00':['superman','0',[7]]}}}},'w':{'i':{'n':{'k':{':':{'\x00':['superwink','2',[2]]}}}}}}}},'r':{'r':{'e':{'n':{'d':{'e':{'r':{':':{'\x00':['surrender','2',[3]]}}}}}}},'f':{'e':{'r':{':':{'\x00':['surfer','0',[4]]}}}}},'z':{'a':{'n':{'n':{'e':{':':{'\x00':['XD','0',[2],':suzanne:']}}}}}},'u':{'s':{':':{'\x00':['hyvesangel','2',[12],':suus:']}}},'n':{':':{'\x00':['sun','2',[8]]}}},'t':{'u':{'p':{'i':{'d':{'m':{'e':{':':{'\x00':['stupidme','0',[3]]}}}}}}}},'w':{'i':{'n':{'g':{'i':{'n':{'g':{':':{'\x00':['swinging','0',[5]]}}}},':':{'\x00':['swing','2',[5]]}}}}}},'r':{'a':{'i':{'n':{'i':{'n':{'g':{':':{'\x00':['raining','1',[8]]}}}}}},'p':{':':{'\x00':['rap','0',[8]]}},'s':{'t':{'a':{':':{'\x00':['rasta','0',[4]]}}}}},'u':{'n':{'f':{'o':{'r':{'e':{'s':{'t':{':':{'\x00':['runforest','0',[8]]}}}}}}}}},'e':{'a':{'d':{'i':{'n':{'g':{':':{'\x00':['reading','0',[8]]}}}}}},'d':{'c':{'a':{'r':{'d':{':':{'\x00':['redcard','2',[7]]}}}}},'r':{'o':{'s':{'e':{'s':{':':{'\x00':['redroses','0',[1]]}}}}}}}},'o':{'c':{'k':{'y':{':':{'\x00':['rocky','2',[7]]}}}},'s':{'e':{':':{'\x00':['rose','2',[1]]}}},'m':{'e':{'o':{':':{'\x00':['romeo','0',[12]]}}}},'l':{'l':{'i':{'n':{'g':{':':{'\x00':['rolling','0',[2]]}}}}}}}},'u':{'m':{'b':{'r':{'e':{'l':{'l':{'a':{':':{'\x00':['umbrella','2',[8]]}}}}}}}},'g':{'o':{'t':{'m':{'a':{'i':{'l':{':':{'\x00':['ugotmail','0',[8]]}}}}}}}}},'t':{'a':{'k':{'e':{'p':{'i':{'c':{'t':{'u':{'r':{'e':{':':{'\x00':['takepicture','0',[8]]}}}}}}}}}},'z':{':':{'\x00':['taz','1',[11]]}},'l':{'k':{'t':{'o':{'t':{'h':{'e':{'h':{'a':{'n':{'d':{':':{'\x00':['talktothehand','0',[12]]}}}}}}}}}}}}},'e':{'r':{'r':{'i':{'f':{'i':{'e':{'d':{':':{'\x00':['terrified','2',[6]]}}}}}}}},'l':{'l':{'j':{'o':{'k':{'e':{':':{'\x00':['telljoke','2',[8]]}}}}}}},'d':{'d':{'y':{'b':{'e':{'a':{'r':{':':{'\x00':['teddybear','1',[11]]}}}}}}}}},'h':{'r':{'e':{'e':{'s':{'i':{'s':{'t':{'e':{'r':{'s':{':':{'\x00':['threesisters','0',[5]]}}}}}}},'o':{'m':{'e':{':':{'\x00':['threesome','0',[1]]}}}}}}},'o':{'w':{'p':{'c':{':':{'\x00':['throwpc','1',[3]]}}}}}},'e':{'h':{'i':{'v':{'e':{'s':{':':{'\x00':['thehives','0',[12]]}}}}}},'b':{'e':{'s':{'t':{':':{'\x00':['thebest','1',[4]]}}}}},'f':{'i':{'s':{'t':{':':{'\x00':['thefist','2',[3]]}}},'n':{'g':{'e':{'r':{':':{'\x00':['thefinger','0',[3]]}}}}}}}},'n':{'x':{':':{'\x00':['thnx','0',[2]]}}}},'o':{'m':{'a':{'t':{'o':{'e':{'s':{':':{'\x00':['tomatoes','1',[6]]}}}}}}},'o':{'h':{'o':{'t':{':':{'\x00':['toohot','0',[8]]}}}},'t':{'h':{':':{'\x00':['tooth','0',[2]]}}}}},'r':{'a':{'m':{'p':{':':{'\x00':['tramp','0',[7]]}}}},'i':{'s':{'t':{'a':{'n':{':':{'\x00':['tristan','2',[12]]}}}}}}},'u':{'r':{'t':{'l':{'e':{':':{'\x00':['turtle','0',[11]]}}}}}},'w':{'o':{'s':{'o':{'m':{'e':{':':{'\x00':['twosome','0',[1]]}}}}}}},'y':{'p':{'i':{'n':{'g':{':':{'\x00':['typing','0',[8]]}}}}}}},'w':{'a':{'i':{'t':{'i':{'n':{'g':{':':{'\x00':['waiting','2',[8]]}}}}}},'s':{'h':{'i':{'n':{'g':{':':{'\x00':['washing','0',[8]]}}}}}},'t':{'c':{'h':{'i':{'n':{'g':{'t':{'v':{':':{'\x00':['watchingtv','0',[8]]}}}}}}}}},'v':{'e':{':':{'\x00':['wave','0',[5]]}}}},'c':{':':{'\x00':['wc','1',[8]]}},'e':{'h':{'y':{'v':{'e':{'n':{':':{'\x00':['wehyven','0',[12]]}}}}}},'d':{'d':{'i':{'n':{'g':{':':{'\x00':['wedding','2',[1]]}}}}}}},'i':{'n':{'k':{'k':{'i':{'s':{'s':{':':{'\x00':['winkkiss','0',[1]]}}}}}}}},'h':{'i':{'s':{'t':{'l':{'e':{':':{'\x00':['whistle','0',[8]]}}}}}},'e':{'e':{'l':{'c':{'h':{'a':{'i':{'r':{':':{'\x00':['wheelchair','0',[7]]}}}}}}}}},'o':{'w':{'h':{'a':{'t':{'w':{'h':{'e':{'r':{'e':{':':{'\x00':['wereldbol','0',[12],':whowhatwhere:']}}}}}}}}}}}},'o':{'u':{'n':{'d':{'e':{'d':{':':{'\x00':['wounded','2',[7]]}}}}}},'r':{'s':{'h':{'i':{'p':{':':{'\x00':['worship','0',[1]]}}}}}},'m':{'a':{'n':{'i':{'n':{'l':{'o':{'v':{'e':{':':{'\x00':['womaninlove','2',[1]]}}}}}}}}}}},'t':{'f':{':':{'\x00':['wtf','0',[3]]}}}},'v':{'i':{'k':{'i':{'n':{'g':{':':{'\x00':['viking','2',[3]]}}}}}},'e':{'l':{'l':{'a':{'h':{':':{'\x00':['hyvesangel','2',[12],':vellah:']}}}}}},'o':{'o':{'d':{'o':{'o':{':':{'\x00':['voodoo','2',[3]]}}}}}}},'y':{'a':{'w':{'n':{':':{'\x00':['yawn','0',[8]]}}}},'e':{'a':{'h':{':':{'\x00':['yeah','1',[2]]}}},'l':{'l':{'o':{'w':{'c':{'a':{'r':{'d':{':':{'\x00':['yellowcard','2',[7]]}}}}}}}}}},'o':{':':{'\x00':['yo','2',[4]]}}},'x':{'d':{':':{'\x00':['XD','0',[2],':xd:']}}},'z':{'z':{'z':{':':{'\x00':['zzz','2',[8]]}}},'o':{'r':{'r':{'o':{':':{'\x00':['zorro','0',[7]]}}}}}},'|':{'\x00':['geschokt','0',[9],':|']}},'=':{':':{')':{'\x00':['bunny2','0',[12],'=:)']}}},'|':{'s':{'\x00':['confused_scheel','0',[6],'|s']},'-':{'s':{'\x00':['confused_scheel','0',[6],'|-s']}}},'^':{'o':{')':{'\x00':['neutraal','0',[9],'^o)']}}}}
;




function Smiley(name, code, extra) {
	this.name = name;
	this.code = code;	
	this.extra = extra;
}

Smiley.prototype.getCode = function() {
	return this.code;
}

Smiley.prototype.getDomSrc = function() {
	return SmileyUtil.getSmileyUrl(this.name);
}

Smiley.prototype.getImgSrcTag = function() {
	return SmileyUtil.getImgSrcTag(this.name);
}

Smiley.prototype.getExtra = function() {
	return this.extra;
}

function SmileyUtil() {
}


 
SmileyUtil.getSmileyUrl = function(name) {
	if(name.indexOf('http://') != -1){
		return name;
	}
	else{
		return hyves_smiley_path + "smiley_" + name + ".gif";
	}
} 

SmileyUtil.getImgSrcTag = function(smileyName) {
	return '<img src="' + SmileyUtil.getSmileyUrl(smileyName) + '" border="0"/>';
} 

var sentinel = "\0";

SmileyUtil._findAll = function(s, callback) {

	//yields all possible non-overlapping matches of the words in the tree with string s
	//returns start, end index and word value"""
	//initialize
	var root = smileyTrie;
	var m = null;
	var i=0;
	s = s + sentinel; //added sentinal otherwise last match not matched

	var shortFound=false;
	var shortM=0;
	var shortI=0;
	var shortData=null;
	var shortEnd=0;

	//all found toikens will be pushed onto this array
	while (i < s.length) {
		var c=s.charAt(i).toLowerCase();
        if (!(c in root)) {
        	if (sentinel in root) {
				//no longer match possible
				//this.debug('longest match found m=' + m + ' i=' + i)
				shortFound = false;
				callback.call(this, m, i, root[sentinel]);
			}
            else if (c == sentinel) {
				//end of string and no match
				//this.debug('match at end of string m=' + m + ' i=' + i)
				shortFound = false;
				callback.call(this, 0, 0, [sentinel]);
			}
			else {
				//no match check if there is a remenbered short match
				if (shortFound) {
					//this.debug('shorter one already matched m=' + m + ' i=' + i)
					callback.call(this, shortM,shortI,shortData);
					shortFound = false;
					//scan from end of last shortmatch
					i = shortEnd-1;
				}
				
			}
			root = smileyTrie;
		}
		if (c in root) {
			if (root == smileyTrie) {
				m = i; //marks the beginning of a possible new match
			}
			if (c == sentinel) {
				//end of string and match
				//this.debug('match at end of string m=' + m + ' i=' + i)
				shortFound = false;
				callback.call(this, m, i, root[sentinel]);
			}
			if (sentinel in root) {
				//this.debug('found short match, continue scanning m=' + m + ' i=' + i)
				//shortmatch found remember and scan along to see if there is longer match
				shortFound = true;
				shortM = m;
				shortI = i;
				shortData = root[sentinel];
				shortEnd = i;
			}
			root = root[c]; //descend down the tree
		}
		i = i + 1;
	}
}

SmileyUtil.log = logging.getLogger("SmileyUtil");

var logCounter = 0;
SmileyUtil.debug = function(msg) {
    //logCounter++;	
    //SmileyUtil.log.debug(msg + ' (' + logCounter + ')');
} 

SmileyUtil.replaceSmilies = function(inputString, aExtra, illegalSmileyCallback) {
	//SmileyUtil.debug('starting smiley replace on ' + inputString);
	//SmileyUtil.debug(aExtra);
	//inputString = escape_html_decode(inputString);
	var offset = 0;
    var aResult = [];
    this._findAll(inputString, function(m, i, data) {
            var smileyName = data[0];
            var smileyPackNeeded = parseInt(data[1]);
            if (smileyName==sentinel) {
                //end of string
                aResult.push(inputString.substring(offset, inputString.length));
            }
            else if (aExtra.indexOf(smileyPackNeeded) == -1) {
            	aResult.push(inputString.substring(offset, m));
            	if (illegalSmileyCallback) {
            		illegalSmileyCallback.call(this, smileyPackNeeded);
            	}
            }
            else {
                aResult.push(inputString.substring(offset, m));
            	if (inputString.substring(m).match(/^[^<]*>/) || // the smilie is inside a tag
            		aResult[aResult.length-1].match(/&#?[a-z0-9]{1,4}$/)  // part of the smiley is part of an escape sequence
		            	) {
            		
	                aResult.push(inputString.substring(m,i));
            	} else {
	                aResult.push(SmileyUtil.getImgSrcTag(data[0]));
            	}
            }
            offset = i;
        });
	//SmileyUtil.debug("result");
	return aResult.join('');
}

SmileyUtil.removeSmilies = function(inputString, aExtraValid, illegalSmileyCallback) {
	
	SmileyUtil.debug('starting smiley remove');
	
	//if no extra given remove all smileys
	if(!aExtraValid) {
		aExtraValid = [];	
	}
	
	var offset = 0;
    var result = '';

    this._findAll(inputString, function(m, i, data) {
    		SmileyUtil.debug('remove smilies match at m=' + m + ' i =' + i);
			var smileyName = data[0];
    		if (smileyName==sentinel) {
    			SmileyUtil.debug('remove smilies sentinel found');
    			result = result + inputString.substring(offset, inputString.length);	
    		} 
    		else if (!(data[1] in aExtraValid)) {
				SmileyUtil.debug('remove smilies match found ' + smileyName);
    			
    			result = result + inputString.substring(offset, m);
            	
            	if (illegalSmileyCallback) {
            		illegalSmileyCallback.call(this, data[1]);
            	}
            }
            else {
                result = result + inputString.substring(offset, i);
            }
            
            offset = i;
        });
	
	SmileyUtil.debug('result from removeSmilies ' + result);
	return result;	
}

//return in the smileys in a dictionary with as keys the categories, used
//to build the rte smilieypicker
SmileyUtil.createSmiliesArray = function() {
	
	if (catSmileyArray != null)	{
		return catSmileyArray
	}
	catSmileyArray = {};
	getSmilieChildren(smileyTrie);
	return catSmileyArray;
}


//get the smilies in the correct categories
//called from the RTE
var catSmileyArray = null;
var alreadyFound = [];
function getSmilieChildren(aSmileyTrie) {
	
	for (c in aSmileyTrie) {
		
		if(typeof(aSmileyTrie[c]) != 'object')
			return;
			
        if ("\0" in aSmileyTrie[c]) {
            var smileyDef = aSmileyTrie[c]["\0"];
            
            //get the array of categories this smiley is in,
            //can be more than 1
            var aCategories = smileyDef[2];
            aCategories.each( function(category, index) {
            		categoryKey = new String(category);
	            	smiliesInCategory = catSmileyArray[categoryKey];
		            if(!smiliesInCategory) {
		                smiliesInCategory = [];
		                catSmileyArray[categoryKey] = smiliesInCategory;   
		            }
		            
		            name = smileyDef[0];
		            extra = smileyDef[1];
		            //check if code is different than default :name:
		            if (smileyDef.length<4) {
		            	code = ':' + name + ':';
		            }
		            else {
		            	code = smileyDef[3];
		            }
		            
					if(alreadyFound.indexOf(name+'_'+categoryKey) == -1){
		           		smiliesInCategory[smiliesInCategory.length] = new Smiley(name, code, extra);	
						alreadyFound.push(name+'_'+categoryKey);
					}	            
		        } );
        }

		getSmilieChildren(aSmileyTrie[c]);

	};
}





function escape_html(text) {
	var i, returntext = [];
	for (i=0;i<text.length;i++)
	{
		ch = text.charAt(i);
		chcode = text.charCodeAt(i);
		if (ch == "&")
			returntext.push("&amp;");
		else if (ch == ">")
			returntext.push("&gt;");
		else if (ch == "<")
			returntext.push("&lt;");
		else if (ch == "\"")
			returntext.push("&quot;");
		else if (chcode > 127)
			returntext.push("&#"+chcode+";");
		else
			returntext.push(ch);
	}
	return returntext.join("");
} 


String.prototype.split = (function (old) {
    return function (delimiter, limit) {
        var result = old.apply(this, arguments);
        
        // If delimiter was RegExp and contained parenthesis,
        // modify the result of the original method to adhere to the standard
        
        if(delimiter instanceof RegExp){
	        //modify result
	        if(	delimiter.source.indexOf("(") != -1 &&
	        	delimiter.source.indexOf(")") != -1 &&
	        	delimiter.source.indexOf("(") < delimiter.source.indexOf(")")
	        ){ // has parenthesis in regex
		        var temp = [];
		        var collect = [];
		        var inEntity = false;
		        for(var i = 0; i < this.length;i++){
		        	if(this.charAt(i) == "&"){
		            	collect.push(temp.join(""));
		            	temp = [];
		            	temp.push(this.charAt(i));
		            	inEntity = true;
		           	}
		           	else if(this.charAt(i) == ";" && inEntity){
		           		temp.push(this.charAt(i));
		           		collect.push(temp.join(""));
		           		temp = [];
		           		inEntity = false;
		           	}
		           	else{
		           	  	temp.push(this.charAt(i));
		           	}
		        }
		        collect.push(temp.join(""));
		        result = collect;
			}
        }
		return result;
    };
})(String.prototype.split);

function escape_html_decode(html) {

	var i, aPart = html.split(/(&.{2,4};)/);

	for (i=1; i<aPart.length; i+=2) {
		switch(aPart[i]) {
			case "&amp;":
				aPart[i]="&";
				break;
			case "&lt;":
				aPart[i]="<";
				break;
			case "&gt;":
				aPart[i]=">";
				break;
			case "&quot;":
				aPart[i]="\"";
				break;
			default:
				aPart[i]=String.fromCharCode(aPart[i].substring(2,-1));
				break;
		}
	}
	return aPart.join("");
} 


/*
 * performs the fancy_layout for a text
 * @param String
 * @return String
 */
findLinksInTextRegExp=null;


ssMediasize = new Object();
ssMediasize[1] = new Object();
ssMediasize[1]['width'] = 50;
ssMediasize[1]['height'] = 50;
ssMediasize[2] = new Object();
ssMediasize[2]['width'] = 75;
ssMediasize[2]['height'] = 75;
ssMediasize[3] = new Object();
ssMediasize[3]['width'] = 120;
ssMediasize[3]['height'] = 120;
ssMediasize[5] = new Object();
ssMediasize[5]['width'] = 500;
ssMediasize[5]['height'] = 500;
ssMediasize[6] = new Object();
ssMediasize[6]['width'] = 700;
ssMediasize[6]['height'] = 700;


function fancy_layout(originaltext, type, mediasize, smiley_extra, illegalSmileyCallback)
{
    var options = {};

	//workaround because we can't strong-type the function
	originaltext = new String(originaltext);
	if (!smiley_extra)
	{
		smiley_extra = [SMILEY_EXTRA_PREMIUM, SMILEY_EXTRA_OPTIN, SMILEY_EXTRA_DEFAULT];
	}
	modifier = "normal"; // default modifier
	// modifiers may be: mobile (format for mobile site), newtarget (all links use target=_blank), nosmilies (all smilies are removed)
	// checks if the $type has a modifier (genarally used in the mobile site
	if(type.indexOf("|") > 0) {
		aType = type.split('|');
		type = aType[0];
		modifier = aType[1];	
	}
    //added option to seperate modifier by comma, client side templates
    //did not like pipe separator
    if(type.indexOf(",") > 0) {
        aType = type.split(',');        
        type = aType[0];
        modifier = aType[1];
        //check if type contains extra options
        if(aType.length > 2) {
            for(var j = 2; j < aType.length; j++) {
                var aOption = aType[j].split('=');
                if(aOption.length == 2) {
                    options[aOption[0]] = aOption[1];  
                }
            }
        }    
    }
	
	if (!ssMediasize[mediasize])
		mediasize = 2;
	
	image_width = ssMediasize[mediasize]['width'];
	image_height = ssMediasize[mediasize]['height'];

	var escapeText = false;
	var smilies = false;
	var links = false;
	var layout = false;
	var pictures = false;
	var hyvers = false;
	var colour = false;
	var nl2br = false;
	var amazon_block = false;
	var amazon_layover = false;
	var gagget = false;
	var external_images = false;
	var voicereaction = false;

	switch (type)
	{
		case "blog":
			external_images = true;
			// deliberate fallthrough
		case "body":
			escapeText = smilies = links = layout = pictures = hyvers = colour = nl2br = amazon_block = amazon_layover = gagget = voicereaction = true;
			break;
		case "oneliner":
			escapeText = smilies = links = layout = pictures = hyvers = colour = amazon_block = true;
			break;
		case "link":
		case "title":
			escapeText = smilies = true;
			break;
		case "nickname":
			escapeText = smilies = layout = true;
			break;
		case "naturalname":
			escapeText = true;
			break;
		case "edit":
			escapeText = true;
			break;
		case "escape":
			escapeText = true;
			break;
		case "hyver":
			escapeText = hyvers = smilies = true;
			break;
		case "amazon":
			escapeText = amazon_layover = amazon_block = smilies = true;
			break;
		case "flashname":
			smilies = true;
			modifier = "nosmilies";
			break;
		default:
			alert("Unknown fancy_layout type: "+type);
			break;
	}
	
	var laidouttext = originaltext;

	if (escapeText)
	{
		laidouttext = escape_html(laidouttext);
	}

	if (nl2br)
	{
		laidouttext = laidouttext.replace(/\r?\n/g,"<br />");
	}
	
	if(gagget) {
		var kGadgets = new Object();
		laidouttext = laidouttext.replace(/\[gadget\](.*?)\[\/gadget\]/gim, function(string, gaggetHtml) {
				gaggetHtml = stripUnicodeCharacters(gaggetHtml);
				// first, we need to de-escape the gagget
				if(nl2br) {
					gaggetHtml = gaggetHtml.replace("<br />", "\n");
				}
				if(escapeText) {
					gaggetHtml = escape_html_decode(gaggetHtml);
				}
				// we need to check the gagget asynchronous, so we'll write a tag now, and insert the gagget later
				if (!window["sGagget"]) {
					window.sGagget = {};
					window.sGaggetCount = 0;
				}
				var id = "js_gagget_"+(window.sGaggetCount++);
				
				var notallowedhtml = templates.process('gadget.msg.error', {id: id, msg: templates.translate('ILLEGAL_GADGET')});
				var allowedhtml;
				if(navigator.userAgent.indexOf("MSIE 6") != -1) {
					allowedhtml = templates.process('gadget.msg.ok', {id: id, msg: templates.translate('RTE_MSIE6_GOODGADGET')});
				} else {
					allowedhtml = gaggetHtml;
				}
				
				if(window.sGagget[gaggetHtml]) {
					if(window.sGagget[gaggetHtml].approved == "approved") {
						var key = "[gadget]" + Math.round(Math.random() * 1000000) + "[/gadget]";
						kGadgets[key] = allowedhtml;
						return key;
					} else if(window.sGagget[gaggetHtml].approved == "not approved") {
						return notallowedhtml;
					} else {
					   // call pending, do nothing
					}
				} else {
					window.sGagget[gaggetHtml] = {
						approved: "pending",
						gaggetids: []
					}
					var parameters = {
                        widgetHtml: gaggetHtml
                    };
                    if(options.maxGadgetWidth) {
                        parameters.maxGadgetWidth = options.maxGadgetWidth;
                    }
					new Ajax.Request(
						'/checkWidget.php',
						{
							method: 'post',
							parameters: parameters,
							onException: function(request, exception) { logging.error("" + exception, exception); },
							onComplete: function(originalRequest) {
								if(originalRequest.responseText.substr(0, 1) != "0") {
									var replaceHtml = originalRequest.responseText;
									window.sGagget[gaggetHtml].approved = "approved"
								} else {
									var replaceHtml = notallowedhtml;
									window.sGagget[gaggetHtml].approved = "not approved"
								}
								for(var i = 0; i < window.sGagget[gaggetHtml].gaggetids.length; i++) {
									logging.debug('now updating element ' + window.sGagget[gaggetHtml].gaggetids[i]);
									Element.update(window.sGagget[gaggetHtml].gaggetids[i], '<div>' + replaceHtml + '</div>');
								}
							}
						}
					);
				}
				// we are pending
				
				window.sGagget[gaggetHtml].gaggetids.push(id);
				return templates.process('gadget.msg.ok', {id: id, msg: templates.translate('RTE_PREVIEW_GADGET')});
			});
	}
	
    if (layout)
    {
          laidouttext = laidouttext.replace(/\[b\](.*?)\[\/b\]/mg,"<strong>$1</strong>"); //process b-tags
          laidouttext = laidouttext.replace(/\[i\](.*?)\[\/i\]/mg,"<em>$1</em>"); //process i-tags
          laidouttext = laidouttext.replace(/\[strike\](.*?)\[\/strike\]/mg,"<span style=\"text-decoration: line-through;\">$1</span>"); //process strike-tags
    }

    if (pictures)
    {
        laidouttext = laidouttext.replace(/\[media id=(\d*)([a-zA-Z0-9-_]{4}) size=small\]/mg,'<a href="http://'+hyves_base_url+'/mediaLink.php?media_id=$1&media_secret=$2"'+(modifier=="newtarget"?' target="_blank"':'')+'><img border="0" src="/showImage.php/$1_75_75_$2.jpeg" style="padding: 5px 3px 3px 0px; float: left;" /></a>');
        laidouttext = laidouttext.replace(/\[media id=(\d*)([a-zA-Z0-9-_]{4}) size=medium\]/mg,'<a href="http://'+hyves_base_url+'/mediaLink.php?media_id=$1&media_secret=$2"'+(modifier=="newtarget"?' target="_blank"':'')+'><img border="0" src="/showImage.php/$1_120_120_$2.jpeg" style="padding: 5px 3px 3px 0px; float: left;" /></a>');
        laidouttext = laidouttext.replace(/\[media id=(\d*)([a-zA-Z0-9-_]{4}) size=large\]/mg,'<a href="http://'+hyves_base_url+'/mediaLink.php?media_id=$1&media_secret=$2"'+(modifier=="newtarget"?' target="_blank"':'')+'><img border="0" src="/showImage.php/$1_200_200_$2.jpeg" style="padding: 5px 3px 3px 0px; float: left;" /></a>');
        laidouttext = laidouttext.replace(/\[media id=(\d*)([a-zA-Z0-9-_]{4}) size=xlarge\]/mg,'<a href="http://'+hyves_base_url+'/mediaLink.php?media_id=$1&media_secret=$2"'+(modifier=="newtarget"?' target="_blank"':'')+'><img border="0" src="/showImage.php/$1_500_500_$2.jpeg" style="padding: 5px 3px 3px 0px; float: left;" /></a>');
        laidouttext = laidouttext.replace(/\[medianolink id=(\d*)([a-zA-Z0-9-_]{4}) size=small\]/mg,'<img border="0" src="/showImage.php/$1_75_75_$2.jpeg" style="padding: 5px 3px 3px 0px; float: left;" />');
        laidouttext = laidouttext.replace(/\[medianolink id=(\d*)([a-zA-Z0-9-_]{4}) size=medium\]/mg,'<img border="0" src="/showImage.php/$1_120_120_$2.jpeg" style="padding: 5px 3px 3px 0px; float: left;" />');
        laidouttext = laidouttext.replace(/\[medianolink id=(\d*)([a-zA-Z0-9-_]{4}) size=large\]/mg,'<img border="0" src="/showImage.php/$1_200_200_$2.jpeg" style="padding: 5px 3px 3px 0px; float: left;" />');
        laidouttext = laidouttext.replace(/\[medianolink id=(\d*)([a-zA-Z0-9-_]{4}) size=xlarge\]/mg,'<img border="0" src="/showImage.php/$1_500_500_$2.jpeg" style="padding: 5px 3px 3px 0px; float: left;" />');

        laidouttext = laidouttext.replace(/\[media id=(\d*)([a-zA-Z0-9-_]{4})\]/mg,'<a href="http://'+hyves_base_url+'/mediaLink.php?media_id=$1&media_secret=$2"'+(modifier=="newtarget"?' target="_blank"':'')+'><img border="0" src="/showImage.php/$1_'+image_width+'_'+image_height+'_$2.jpeg" align="left" style="padding: 5px 3px 3px 0px;" /></a>');
        laidouttext = laidouttext.replace(/\[medianolink id=(\d*)([a-zA-Z0-9-_]{4})\]/mg,'<img border="0" src="/showImage.php/$1_'+image_width+'_'+image_height+'_$2.jpeg" align="left" style="padding: 5px 3px 3px 0px;" />');
    }


	if (voicereaction)
	{
	    laidouttext = laidouttext.replace(/\[voicereaction id=(\d*)([a-zA-Z0-9-_]{4})\]/mg, function(origString, mediaId, mediaSecret){
			// Create the global object if not already present
			if (!window["sVoiceReaction"]) {
				window.sVoiceReaction = {};
			}

	    	// check the cache to see if we have already done an AJAX call before 
			if (window.sVoiceReaction[origString]){
				return window.sVoiceReaction[origString];
			} else {
				// do the ajax call 
				var parameters = {
					module: 'media',
					action: 'getVoiceReactionHTML',
					mediaid: mediaId,
					mediasecret: mediaSecret,
					xmlHttp: 1
				}; 
				
				var savedKey = origString;
				var randomNum = Math.floor(Math.random()*100);
				var spanId = 'voiceReactionSpan_'+mediaId+'_'+randomNum;
				
				origString = '<span id="'+spanId+'"></span>';
				new Ajax.Updater(spanId,'/index.php',{ 
					method: 'get',
					parameters: parameters,
					onComplete: function(returnData){
						window.sVoiceReaction[savedKey] = '<span id="'+spanId+'">'+returnData.responseText+'</span>';		
					}
				});
				return origString;
			}
	    });
	}


    if (hyvers)
    {
        laidouttext = laidouttext.replace(/\[hyver=([a-zA-Z0-9-_]*)\](.*?)\[\/hyver\]/mg,'<a href="http://$1.'+hyves_base_url+'/">$2</a>');
    }

    if (amazon_layover)
    {
           if(modifier == "mobile") {
                 // this option doesn't make a lot of sense since mobile users don't use javacsript
           }
           else
           {
                 laidouttext = laidouttext.replace(/\[amznlo=([a-zA-Z0-9]*)_([a-zA-Z0-9]*)_([a-zA-Z0-9]*)\](.*?)\[\/amznlo\]/mg,'<span class="memberlayover_container" onmouseover="fancy_layout_amazonlayover_start(this);" onmouseout="fancy_layout_amazonlayover_stop(this);"><a href="/index.php?l1=fr&l2=amazon&l3=show&asin=$1&country=$2category=$3" class="memberlayover_link"'+(modifier=="newtarget"?' target="_blank"':'')+'>$4</a></a><a class="amazonlayover_layover" name="amazonlayover_$1_$2_$3"></a></span>');                  
           }
    }

    if (amazon_block)
    {
           if(modifier == "mobile") {
                 // this option doesn't make a lot of sense since mobile users don't use javacsript
           }
           else
           {
                 laidouttext = laidouttext.replace(/\[amznblk=([a-zA-Z0-9]*)_([a-zA-Z0-9]*)_([a-zA-Z0-9]*)\](.*?)\[\/amznblk\]/mg, generate_amazon_block_table('$1', '$2', '$3', '$4'));                  
           }
    }
 
	if( external_images ) {
		laidouttext = laidouttext.replace(/\[img\](https?:\/\/[a-z0-9:\/#@{};|~&%._+?=\!\-]*?)\[\/img\]/gi, ' <img src="$1" border="0" /> ');
	}

    if (links)
	{
		if (!findLinksInTextRegExp)
		{
			var aMatch = new Array();

			var replacefunction = function(match, link, content, startpos) {
					aMatch.push([[match, startpos],[link],[content]]);
					return match;
				}

			var replacefunctiontarget = function(match, link, target, content, startpos) {
					aMatch.push([[match, startpos],[link],[target],[content]]);
					return match;
				}

			var replacefunctionwithcharacterbeforecheckhttp = function(match, link, content, startpos) {
					if (startpos > 0) {
						var c = laidouttext.charAt(startpos-1)
						if (c == "'" || c == '"' || c == "=") {
							return match;
						}
					}
					return replacefunction(match, link, content, startpos)
				}
			var replacefunctionwithcharacterbeforecheckwww = function(match, link, content, startpos) {
					if (startpos > 0) {
						var c = laidouttext.charAt(startpos-1)
						if (c == "'" || c == '"' || c == "/" || c == "=") {
							return match;
						}
					}
					return replacefunction(match, link, content, startpos)
				}
			var replacefunctionwithcharacterbeforecheckhyvesnl = function(match, link, content, startpos) {
					if (startpos > 0) {
						var c = laidouttext.charAt(startpos-1)
						if (c == "'" || c == '"' || c == "/" || c == "=" || c == "." || c == "-" || c == "_") {
							return match;
						}
					}
					return replacefunction(match, link, content, startpos)
				}
				
			laidouttext.replace(/\[url=([^\] ]*) target=(\w*)\](.*?)\[\/url\]/igm, replacefunctiontarget);
			laidouttext.replace(/\[url=([^\] ]*)\](.*?)\[\/url\]/igm, replacefunction);
			laidouttext.replace(/\[url\]((.*?))\[\/url\]/igm, replacefunction);
			laidouttext.replace(/\b((https?:\/\/[^\s\[<]*))/igm, replacefunctionwithcharacterbeforecheckhttp);
			laidouttext.replace(/\b((www.[^\s\[<]*))/igm, replacefunctionwithcharacterbeforecheckwww);
			laidouttext.replace(/\b(([a-z0-9-_]+\.hyves\.[^\s\[<]*))/igm, replacefunctionwithcharacterbeforecheckhyvesnl);
			
			
			asToReplace = new Array(); // will contain start, end, url and content of match
			for(var i=0; i < aMatch.length; i++) {
				var match = aMatch[i];
				sToReplace = {};
				sToReplace["code"]=match[0][0];
				sToReplace["start"]=0+match[0][1];
				sToReplace["end"]=sToReplace["start"]+match[0][0].length;
				sToReplace["link"]=match[1][0];
				if (match.length == 4) { //we got a target
					sToReplace["target"]=match[2][0];
					sToReplace["content"]=match[3][0];
				} else {
					sToReplace["content"]=match[2][0];
				}
				
				if (!sToReplace["target"] || sToReplace["target"] === "") {
					sToReplace["target"]="_blank";
				}

				sToReplace["order"]=sToReplace["start"]+1/sToReplace["end"]; // this allows for easy searching, first start, longest match first
				asToReplace.push(sToReplace);
			}
			
			asToReplace.sort(function(a,b) {return a["order"]-b["order"]});
			
			var strpointer=0;
			var result = "";
			var tagcount = 0;
			for(var i=0;i < asToReplace.length; i++) {
				sToReplace=asToReplace[i];
				if (sToReplace["start"] < strpointer) {
					// overlap, we don't care about this one, since it is already replaced
					continue;
				}
				var strtoadd = laidouttext.substr(strpointer, sToReplace["start"]-strpointer);
				result += strtoadd
				var opentags = strtoadd.match(/</g);
				var closetags = strtoadd.match(/>/g);
				tagcount += (opentags?opentags.length:0)-(closetags?closetags.length:0);
				var link;
				if (sToReplace["link"].substr(0, 7) == "http://" || sToReplace["link"].substr(0, 8) == "https://" ) {
					link = sToReplace["link"];
				} else {
					link = "http://"+sToReplace["link"];
				}
				
				if (tagcount > 0) {
					var strtoadd = sToReplace["code"];
				} else {
					var strtoadd = '<a class="personal_link" href="'+link+'" target="'+sToReplace["target"]+'">'+sToReplace["content"]+'</a>';
				}
				result += strtoadd
				var opentags = strtoadd.match(/</g);
				var closetags = strtoadd.match(/>/g);
				tagcount += (opentags?opentags.length:0)-(closetags?closetags.length:0);
				strpointer=sToReplace["end"];
			}
			result += laidouttext.substr(strpointer);
			
			laidouttext = result;
		}
	}

	if (smilies)
	{
		if(modifier == "nosmilies") {
			laidouttext = SmileyUtil.removeSmilies(laidouttext);
		} else {
			laidouttext = SmileyUtil.replaceSmilies(laidouttext, smiley_extra, illegalSmileyCallback);
		}
	}
	
	if(gagget) {
		for(var key in kGadgets) {
			laidouttext = laidouttext.replace(key, kGadgets[key]);
		}
	}
	
	return laidouttext;
}


function generate_amazon_block_table(asin, country, category, text)
{
	var strReturn = '<table cellpadding="0" cellspacing="0" border="0">';
	strReturn += '<tr><td valign="top" style="padding-right: 10px;" class="personal_text"><a href="/index.php?l1=fr&l2=amazon&l3=show&asin='+asin+'&country='+country+'&category='+category+'"><img src="http://images.amazon.com/images/P/'+asin+'.03._SCTHUMBZZZ_.jpg" border=0></a></td>';
	strReturn += '<td valign="top" class="personal_text">'+text+'</td></tr></table><br>';
	return strReturn;
}





sTextareaSelectionTextRange = new Object();

//this function is needed since IE doesn't save its position on loosing focus
function textarealoosesfocus(name)
{
	textarea = document.getElementById(name+'_code');
	if (textarea.selectionStart != null && textarea.selectionStart != undefined) // do this the mozilla way
	{
	}
	else
	{
		selrange=document.selection.createRange().duplicate();
		sTextareaSelectionTextRange[name] = selrange;
	}
}

function fancylayout_blocktag(starttag, endtag, name)
{
	textarea = document.getElementById(name+'_code');
	if (textarea.selectionStart != null && textarea.selectionStart != undefined) // do this the mozilla way
	{
		selectionstart = textarea.selectionStart;
		selectionend = textarea.selectionEnd;
		text = textarea.value;
		text = text.substr(0,selectionstart)+starttag+text.substr(selectionstart);
		selectionstart+=starttag.length;
		selectionend+=starttag.length;
		text = text.substr(0,selectionend)+endtag+text.substr(selectionend);
		textarea.value=text;
		textarea.selectionStart = selectionstart;
		textarea.selectionEnd = selectionend;
	}
	else if (document.selection && document.selection.createRange())// use the IE way
	{
		textarea.focus();
		selrange=document.selection.createRange().duplicate();
		if (selrange.text.length == 0)
		{
			if (sTextareaSelectionTextRange[name] != null)
			{
				selrange = sTextareaSelectionTextRange[name]; 
			}
		}
		blockselect = (selrange.text.length > 0);
		if (selrange.parentElement() == textarea)
		{
			selrange.text=starttag+selrange.text+endtag;
			// for some strange reason, we have to get the selrange again, becasue it has been reset
			if (blockselect)
			{
				selrange=document.selection.createRange().duplicate();
				selrange.moveStart("character",starttag.length);
				selrange.moveEnd("character",-endtag.length);
				selrange.select();
			}
			else
			{
				selrange=document.selection.createRange().duplicate();
				selrange.moveEnd("character",-endtag.length);
				selrange.select();
			}
		}
	}
	else // unsupported browser
	{
		textarea.value+=starttag+endtag;
	}
	textarea.focus();
}

function fancylayout_inserttext(texttoinsert, name)
{
	textarea = document.getElementById(name+'_code');
	if (textarea.selectionStart != null && textarea.selectionStart != undefined) // do this the mozilla way
	{
		text = textarea.value;
		selectionstart = textarea.selectionStart;
		selectionend = textarea.selectionEnd;
		text = text.substr(0,selectionstart)+texttoinsert+text.substr(selectionend);
		selectionstart+=texttoinsert.length;
		selectionend=selectionstart;
		textarea.value=text;
		textarea.selectionStart = selectionstart;
		textarea.selectionEnd = selectionend;
	}
	else if (document.selection && document.selection.createRange())// use the IE way
	{
		textarea.focus();
		selrange=document.selection.createRange().duplicate();
		if (selrange.text.length == 0)
		{
			if (sTextareaSelectionTextRange[name] != null)
			{
				selrange = sTextareaSelectionTextRange[name]; 
			}
		}
		selrange.text=texttoinsert;
		selrange.select();
	}
	else // unsupported browser
	{
		textarea.value+=texttoinsert;
	}
	textarea.focus();
}

function fancylayout_getselection(name)
{
	textarea = document.getElementById(name+'_code');
	if (textarea.selectionStart != null && textarea.selectionStart != undefined) // do this the mozilla way
	{
		text = textarea.value;
		selectionstart = textarea.selectionStart;
		selectionend = textarea.selectionEnd;
		return text.substring(selectionstart,selectionend);
	}
	else if (document.selection && document.selection.createRange())// use the IE way
	{
		textarea.focus();
		selrange=document.selection.createRange().duplicate();
		if (selrange.text.length == 0)
		{
			if (sTextareaSelectionTextRange[name] != null)
			{
				selrange = sTextareaSelectionTextRange[name]; 
			}
		}		
		return selrange.text;
	}
	else // unsupported browser
	{
		return "";
	}
	
}

function fancy_layout_memberlayover_handleXMLResponse(response) {
	
	var reply = response.responseXML.documentElement;
	var link = reply.getElementsByTagName('link')[0].firstChild.nodeValue;
	var mediaUrl = reply.getElementsByTagName('media_url')[0].firstChild.nodeValue;
	var nickname = reply.getElementsByTagName('nickname')[0].firstChild.nodeValue;
	var numFriends = reply.getElementsByTagName('nr_friends')[0].firstChild.nodeValue;
	var firstName = reply.getElementsByTagName('firstname')[0].firstChild.nodeValue;
	var lastName = reply.getElementsByTagName('lastname')[0].firstChild ? reply.getElementsByTagName('lastname')[0].firstChild.nodeValue : '';
	
	var html = '';
	html += '<table bgcolor="#FFCC66" style="border: 1px solid #6699CC" width="200" height="80"><tr><td style="padding-right: 5px;" class="BodyTextSmall" valign="top">';
	html +=   '<a href="' + link + '"><img border="0" src="' + mediaUrl + '"></a><br>';
	html +=   '<a href="' + link + '">' + fancy_layout(nickname, 'nickname', 1) + ' (' + numFriends + ')' + '</a>';
	html += '</td><td valign="top">';
	html +=   '<a href="' + link + '">' + fancy_layout(firstName + ' ' + lastName, 'naturalname', 1) + '</a>';
	html += '</td></tr></table>';
	
	var username = reply.attributes.getNamedItem('member_username').value;
	var aElements = document.getElementsByName('memberlayover_' + username);
	for(var i = 0; i < aElements.length; i++) {
		aElements[i].innerHTML = html;
	}
}

function fancy_layout_amazonlayover_handleXMLResponse(response) {
	var reply = response.responseXML.documentElement;
	var status = reply.attributes.getNamedItem('status').value;
	var asin = reply.attributes.getNamedItem('asin').value;
	var country = reply.attributes.getNamedItem('country').value;
	var category = reply.attributes.getNamedItem('category').value;
	
	var link = '/index.php?l1=fr&l2=amazon&l3=show&asin=' + asin + '&country=' + country + '&category=' + category;
	var imageUrl = reply.getElementsByTagName('image_url')[0].firstChild.nodeValue;
	var imageWidth = reply.getElementsByTagName('image_width')[0].firstChild.nodeValue;
	var imageHeight = reply.getElementsByTagName('image_height')[0].firstChild.nodeValue;
	var title = reply.getElementsByTagName('title')[0].firstChild.nodeValue;
	var authors = reply.getElementsByTagName('authors')[0].firstChild ? reply.getElementsByTagName('authors')[0].firstChild.nodeValue : '';
	var actors = reply.getElementsByTagName('actors')[0].firstChild ? reply.getElementsByTagName('actors')[0].firstChild.nodeValue : '';
	var creators = reply.getElementsByTagName('creators')[0].firstChild ? reply.getElementsByTagName('creators')[0].firstChild.nodeValue : '';
	var manufacturers = reply.getElementsByTagName('manufacturers')[0].firstChild ? reply.getElementsByTagName('manufacturers')[0].firstChild.nodeValue : '';
	var directors = reply.getElementsByTagName('directors')[0].firstChild ? reply.getElementsByTagName('directors')[0].firstChild.nodeValue : '';
	var productGroup = reply.getElementsByTagName('productgroup')[0].firstChild.nodeValue;
	
	var html = '';
	if(status == 1) {
		html += '<table bgcolor="#FFCC66" style="border: 1px solid #6699CC" width="200" height="80"><tr><td style="padding-right: 5px; vertical-align: top;" class="BodyTextSmall">'
		html +=   '<a href="' + link + '"><img border="0" src="' + imageUrl + '" width="' + imageWidth + '" height="' + imageHeight + '"></a>';
		html += '</td><td style="vertical-align: top;">';
		html +=   '<a href="' + link + '"><b>' + title + '</b></a><br>';
		authors ? html += 'Authors: ' + authors + '<br>' : null;
		actors ? html += 'Actors: ' + actors + '<br>' : null;
		creators ? html += 'Creators: ' + creators + '<br>' : null;
		manufacturers ? html += 'Manufacturers: ' + manufacturers + '<br>' : null;
		directors ? html += 'Directors: ' + directors + '<br>' : null;
		html += 'Product group: ' + productGroup + '<br>';
		html += '</td></tr></table>';
	} else {
		html = 'failed.';
	}
	
	var aElements = document.getElementsByName('amazonlayover_' + asin + '_' + country + '_' + category);
	for(var i = 0; i < aElements.length; i++) {
		aElements[i].innerHTML = html;
	}
}

function fancy_layout_memberlayover_stop(el)
{
	el.lastChild.style.display="none";
	el.style.zIndex = "0";
}

function fancy_layout_amazonlayover_stop(el)
{
	el.lastChild.style.display="none";
	el.style.zIndex = "0";
}

function fancy_layout_memberlayover_start(element) {
	
	element.style.zIndex = 1;
	element.lastChild.style.display = 'block';
	
	var name = element.lastChild.attributes.getNamedItem('name').value;
	var username = name.substring(name.lastIndexOf('_') + 1);
	if(element.lastChild.childNodes.length == 0) {
		element.lastChild.innerHTML = '<span></span>';
		
		new Ajax.Request('/a2a.php', {
			method: 'get',
			parameters: {
				method: 'getmembermicroprofile',
				member_username: username
			},
			onSuccess: fancy_layout_memberlayover_handleXMLResponse
		});
	}
}

function fancy_layout_amazonlayover_start(element) {
	
	element.style.zIndex = 1;
	element.lastChild.style.display = 'block';
	
	var tagCode = element.lastChild.attributes.getNamedItem('name').value;
	var aCodes = tagCode.split('_');
	if(element.lastChild.childNodes.length == 0) {
		element.lastChild.innerHTML = '<span></span>';
		
		new Ajax.Request('/a2a.php', {
			method: 'get',
			parameters: {
				method: 'getamazonitem_layover',
				asin: aCodes[1],
				country: aCodes[2],
				category: aCodes[3]
			},
			onSuccess: fancy_layout_amazonlayover_handleXMLResponse
		});
	}
}


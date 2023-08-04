from k4FWCore.parseArgs import parser
parser.add_argument("-f", "--foo", type=int, help="hello world")
my_opts = parser.parse_known_args()

print(f"The answer is {my_opts[0].foo}")
var app = angular.module('eventsApp', ['ngRoute']).
  config(function($locationProvider) {
  $locationProvider.html5Mode(true);
}).
  run(function($rootScope, $location) {
  // get initial state from pebble
  try {
    $rootScope.vibrate = JSON.parse($location.search()['vibrate']);
  } catch(e) {
    $rootScope.vibrate = false;
  }
  try {
    $rootScope.events = JSON.parse($location.search()['events']) || [];
    angular.forEach( $rootScope.events,function(value,index){
      delete value["$$hashKey"];
    })
  } catch(e) {
    $rootScope.events = [];
  }
  // clear URL for routing
  $location.replace();
  $location.search('vibrate', null);
  $location.search('events', null);
});

app.config(function($routeProvider) {
  $routeProvider
  .when('/', {
    controller:'ListCtrl',
    templateUrl:'/list.html'
  })
  .when('/edit/:index', {
    controller:'EditCtrl',
    templateUrl:'/detail.html'
  })
  .when('/new', {
    controller:'CreateCtrl',
    templateUrl:'/detail.html'
  })
  .otherwise({
    redirectTo:'/'
  });
});

app.controller('ListCtrl',  ['$scope', '$rootScope', function($scope, $rootScope){
  $scope.events = $rootScope.events;
  $scope.vibrate = $rootScope.vibrate;

  $scope.save = function () {
    document.location = 'pebblejs://close#' + encodeURIComponent(JSON.stringify( { events: $scope.events, vibrate: $scope.vibrate } ));
  };
}]);

app.controller('EditCtrl', function($scope, $rootScope, $routeParams, $location) {
  $scope.events = $rootScope.events;
  $scope.event = $scope.events[$routeParams.index];

  $scope.destroy = function(item) {
    var index=$scope.events.indexOf(item);
    $scope.events.splice(index,1);
    $location.path('/');
  };

  $scope.save = function() {
    $location.path('/');
  };
});

app.controller('CreateCtrl', function($scope, $rootScope,  $location) {
  $scope.save = function() {
    $rootScope.events.push($scope.event);
    $location.path('/');
  };
});
